/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "catapult/thread/Scheduler.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/utils/AtomicIncrementDecrementGuard.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/SchedulerTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/WaitFunctions.h"
#include "tests/TestHarness.h"
#include <boost/asio/steady_timer.hpp>
#include <thread>

namespace catapult { namespace thread {

#define TEST_CLASS SchedulerTests

	namespace {
		const uint32_t Num_Default_Threads = test::GetNumDefaultPoolThreads();
		constexpr int Wait_Duration_Millis = 5;

		Task CreateContinuousTask(uint64_t startDelayMs = 10) {
			return {
				utils::TimeSpan::FromMilliseconds(startDelayMs),
				CreateUniformDelayGenerator(utils::TimeSpan::FromMilliseconds(10)),
				[]() { return make_ready_future(TaskResult::Continue); },
				"continuous task"
			};
		}

		Task CreateImmediateTask(const TaskCallback& callback) {
			return {
				utils::TimeSpan::FromMilliseconds(0),
				CreateUniformDelayGenerator(utils::TimeSpan::FromMilliseconds(0)),
				callback,
				"immediate task"
			};
		}

		void WaitForScheduled(Scheduler& scheduler, uint32_t numScheduledTasks) {
			WAIT_FOR_VALUE_EXPR(numScheduledTasks, scheduler.numScheduledTasks());
		}

		void WaitForExecuting(Scheduler& scheduler, uint32_t numExecutingTaskCallbacks) {
			WAIT_FOR_VALUE_EXPR(numExecutingTaskCallbacks, scheduler.numExecutingTaskCallbacks());
		}

		class PoolSchedulerPair {
		public:
			explicit PoolSchedulerPair(std::unique_ptr<IoThreadPool>&& pPool)
					: m_pPool(std::move(pPool))
					, m_pScheduler(CreateScheduler(*m_pPool))
			{}

			~PoolSchedulerPair() {
				stopAll();
			}

		public:
			void stopAll() {
				// shutdown order is important
				// 1. m_pScheduler->shutdown cancels all timers, which allows all threads to complete
				// 2. m_pPool->join waits for threads to complete but must finish before m_pScheduler is destroyed
				m_pScheduler->shutdown();
				test::WaitForUnique(m_pScheduler, "m_pScheduler");
				m_pPool->join();
			}

		public:
			Scheduler& operator*() {
				return *m_pScheduler;
			}

			Scheduler* operator->() {
				return m_pScheduler.get();
			}

		private:
			std::unique_ptr<IoThreadPool> m_pPool;
			std::shared_ptr<Scheduler> m_pScheduler;
		};

		PoolSchedulerPair CreateScheduler() {
			return PoolSchedulerPair(test::CreateStartedIoThreadPool());
		}

		// region [Scheduler|Blocking|NonBlocking]Work

		enum class WaitStrategy { WaitDuringPost, NoWaitDuringPost };

		class SchedulerWork {
		private:
			class State : public std::enable_shared_from_this<State> {
			public:
				State(const test::WaitFunction& wait, WaitStrategy waitStrategy, boost::asio::io_context& ioContext)
						: m_wait(wait)
						, m_waitStrategy(waitStrategy)
						, m_ioContext(ioContext)
						, m_isPostingWork(false)
						, m_shouldWait(true)
				{}

			public:
				void isPostingWork(bool value) {
					m_isPostingWork = value;
				}

				void unblock() {
					m_shouldWait = false;
				}

				future<TaskResult> wait() {
					auto pPromise = std::make_shared<promise<TaskResult>>();
					m_wait(m_ioContext, [pThis = shared_from_this(), pPromise]() {
						if (pThis->shouldWait())
							return true;

						CATAPULT_LOG(debug) << "done waiting";
						pPromise->set_value(TaskResult::Continue);
						return false;
					});

					return pPromise->get_future();
				}

			private:
				bool shouldWait() const {
					return (WaitStrategy::WaitDuringPost == m_waitStrategy || !m_isPostingWork) && m_shouldWait;
				}

			private:
				test::WaitFunction m_wait;
				WaitStrategy m_waitStrategy;
				boost::asio::io_context& m_ioContext;
				std::atomic_bool m_isPostingWork;
				std::atomic_bool m_shouldWait;
			};

		public:
			SchedulerWork(const test::WaitFunction& wait, WaitStrategy waitStrategy)
					: m_pPool(test::CreateStartedIoThreadPool(1))
					, m_pState(std::make_shared<State>(wait, waitStrategy, m_pPool->ioContext()))
			{}

			~SchedulerWork() {
				// unblock all tasks and wait for them to drain
				m_pState->unblock();
				m_pPool->join();
			}

		public:
			void post(Scheduler& scheduler, uint32_t numPosts) {
				// depending on the type of work there are different waiting strategies for the tasks:
				// BlockingWork: allow all work to be posted to the scheduler BEFORE blocking, otherwise the WaitBlocking test
				//               is timing dependent (depending on how many were posted before the threads started wait blocking)
				// NonBlockingWork: tasks are not completing until the test is over in order to prevent any task from decrementing the
				//                  m_numExecutingTaskCallbacks member of the scheduler which is used in an assert
				m_pState->isPostingWork(true);

				for (auto i = 0u; i < numPosts; ++i)
					postOne(scheduler);

				WaitForScheduled(scheduler, numPosts);
				m_pState->isPostingWork(false);
			}

			void postOne(Scheduler& scheduler) {
				auto task = CreateImmediateTask([pState = m_pState]() {
					return pState->wait();
				});

				scheduler.addTask(task);
			}

		private:
			std::unique_ptr<IoThreadPool> m_pPool;
			std::shared_ptr<State> m_pState;
		};

		class BlockingWork : public SchedulerWork {
		public:
			BlockingWork() : SchedulerWork(test::CreateSyncWaitFunction(Wait_Duration_Millis), WaitStrategy::NoWaitDuringPost)
			{}
		};

		class NonBlockingWork : public SchedulerWork {
		public:
			NonBlockingWork() : SchedulerWork(test::CreateAsyncWaitFunction(Wait_Duration_Millis), WaitStrategy::WaitDuringPost)
			{}
		};

		// endregion
	}

	// region basic

	TEST(TEST_CLASS, SchedulerInitiallyHasNoWork) {
		// Act: set up a scheduler
		auto pScheduler = CreateScheduler();

		// Assert: no work is present
		EXPECT_EQ(0u, pScheduler->numScheduledTasks());
		EXPECT_EQ(0u, pScheduler->numExecutingTaskCallbacks());
	}

	// endregion

	// region shutdown

	namespace {
		void AssertCanShutdownScheduler(size_t numShutdownCalls) {
			// Arrange: set up a scheduler
			auto pScheduler = CreateScheduler();

			// Act: stop the scheduler
			for (auto i = 0u; i < numShutdownCalls; ++i)
				pScheduler->shutdown();

			// Assert: all tasks have been stopped
			EXPECT_EQ(0u, pScheduler->numScheduledTasks());
			EXPECT_EQ(0u, pScheduler->numExecutingTaskCallbacks());
		}
	}

	TEST(TEST_CLASS, SchedulerShutdownSucceedsWhenSchedulerHasNoTasks) {
		AssertCanShutdownScheduler(1);
	}

	TEST(TEST_CLASS, SchedulerShutdownIsIdempotent) {
		AssertCanShutdownScheduler(3);
	}

	TEST(TEST_CLASS, SchedulerCannotAcceptNewTasksAfterShutdown) {
		// Arrange: set up a scheduler
		auto pScheduler = CreateScheduler();

		// - stop the scheduler
		pScheduler->shutdown();

		// Act + Assert: add a task
		EXPECT_THROW(pScheduler->addTask(CreateContinuousTask()), catapult_runtime_error);
	}

	// endregion

	// region shutdown - non-executing tasks

	TEST(TEST_CLASS, SchedulerCanShutdownWithWaitingTasks) {
		// Arrange: set up a scheduler and add a task that executes (30s) in the future
		auto pScheduler = CreateScheduler();
		pScheduler->addTask(CreateContinuousTask(30'000));
		WaitForScheduled(*pScheduler, 1);

		// - stop the scheduler
		pScheduler.stopAll();

		// Assert: all tasks have been stopped (and the waiting task did not execute)
		EXPECT_EQ(0u, pScheduler->numScheduledTasks());
		EXPECT_EQ(0u, pScheduler->numExecutingTaskCallbacks());
	}

	// endregion

	// region shutdown - executing tasks

	namespace {
		template<typename TWaitFunction>
		void AssertSchedulerShutdownDoesNotAbortExecutingCallbacks(TWaitFunction wait) {
			// Arrange: set up a scheduler
			std::atomic_bool isAccepted(false);
			std::atomic<uint32_t> numWaits(0);
			std::atomic<uint32_t> maxWaits(10000);

			auto pPool = utils::UniqueToShared(test::CreateStartedIoThreadPool(1));
			auto pScheduler = CreateScheduler();
			auto task = CreateImmediateTask([&, wait, pPool]() {
				isAccepted = true;
				auto pPromise = std::make_shared<promise<TaskResult>>();
				wait(pPool->ioContext(), [&, pPromise]() {
					if (numWaits < maxWaits) {
						++numWaits;
						return true;
					}

					pPromise->set_value(TaskResult::Break);
					return false;
				});

				return pPromise->get_future();
			});

			// - wait for the callback to be entered
			pScheduler->addTask(task);
			WaitForScheduled(*pScheduler, 1);
			WAIT_FOR(isAccepted);

			// Act: stop the scheduler
			uint32_t preShutdownWaits = numWaits;
			maxWaits = numWaits + 10;
			pScheduler.stopAll();

			// Assert: the callback was allowed to complete and was not aborted
			CATAPULT_LOG(debug)
				<< "preShutdownWaits " << preShutdownWaits
				<< " numWaits " << numWaits
				<< " maxWaits " << maxWaits;
			EXPECT_LE(10u, maxWaits - preShutdownWaits);
			EXPECT_EQ(maxWaits, numWaits);
		}
	}

	TEST(TEST_CLASS, SchedulerShutdownDoesNotAbortExecutingBlockingCallbacks) {
		AssertSchedulerShutdownDoesNotAbortExecutingCallbacks(test::CreateSyncWaitFunction(Wait_Duration_Millis));
	}

	TEST(TEST_CLASS, SchedulerShutdownDoesNotAbortExecutingNonBlockingCallbacks) {
		AssertSchedulerShutdownDoesNotAbortExecutingCallbacks(test::CreateAsyncWaitFunction(Wait_Duration_Millis));
	}

	// endregion

	// region Wait[Non]Blocking

	TEST(TEST_CLASS, SchedulerWorkerThreadsCannotServiceAdditionalRequestsWhenHandlersWaitBlocking) {
		// Arrange: set up a scheduler
		auto pScheduler = CreateScheduler();

		// - post 2X work items on the pool (blocking)
		CATAPULT_LOG(debug) << ">>> posting blocking work";
		BlockingWork work;
		work.post(*pScheduler, 2 * Num_Default_Threads);
		CATAPULT_LOG(debug) << "<<< all work posted";

		// Act: wait for all tasks to be scheduled and executing
		WaitForScheduled(*pScheduler, 2 * Num_Default_Threads);
		WaitForExecuting(*pScheduler, Num_Default_Threads);

		// - wait a bit to give the scheduler time to handle more tasks if there is a bug in the implementation
		test::Pause();

		// Assert: only X / 2X work items are executing
		EXPECT_EQ(2 * Num_Default_Threads, pScheduler->numScheduledTasks());
		EXPECT_EQ(Num_Default_Threads, pScheduler->numExecutingTaskCallbacks());
	}

	TEST(TEST_CLASS, SchedulerWorkerThreadsCanServiceAdditionalRequestsWhenHandlersWaitNonBlocking) {
		// Arrange: set up a scheduler
		auto pScheduler = CreateScheduler();

		// - post 2X work items on the pool (non-blocking)
		CATAPULT_LOG(debug) << ">>> posting non-blocking work";
		NonBlockingWork work;
		work.post(*pScheduler, 2 * Num_Default_Threads);
		CATAPULT_LOG(debug) << "<<< all work posted";

		// Act: wait for all tasks to be scheduled and executing
		WaitForScheduled(*pScheduler, 2 * Num_Default_Threads);
		WaitForExecuting(*pScheduler, 2 * Num_Default_Threads);

		// - wait a bit to give the scheduler time to handle more tasks if there is a bug in the implementation
		test::Pause();

		// Assert: all 2X / 2X work items are executing
		EXPECT_EQ(2 * Num_Default_Threads, pScheduler->numScheduledTasks());
		EXPECT_EQ(2 * Num_Default_Threads, pScheduler->numExecutingTaskCallbacks());
	}

	// endregion

	// region addTask

	TEST(TEST_CLASS, CanAddTask) {
		// Arrange: create a scheduler
		auto pScheduler = CreateScheduler();

		// Act: add a single task
		pScheduler->addTask(CreateContinuousTask(1000));
		WaitForScheduled(*pScheduler, 1);

		// Assert: the task should be scheduled
		EXPECT_EQ(1u, pScheduler->numScheduledTasks());
		EXPECT_EQ(0u, pScheduler->numExecutingTaskCallbacks());
	}

	TEST(TEST_CLASS, CanAddMultipleTasks) {
		// Arrange: create a scheduler
		auto pScheduler = CreateScheduler();

		// Act: add multiple tasks
		for (auto i = 0u; i < 101; ++i)
			pScheduler->addTask(CreateContinuousTask(1000));

		WaitForScheduled(*pScheduler, 101);

		// Assert: all tasks should be scheduled
		EXPECT_EQ(101u, pScheduler->numScheduledTasks());
		EXPECT_EQ(0u, pScheduler->numExecutingTaskCallbacks());
	}

	// endregion

	// region TaskResult::Break

	TEST(TEST_CLASS, TaskIsExecutedUntilBreak) {
		// Arrange: create a scheduler
		auto pScheduler = CreateScheduler();

		// Act: add a single task with a break
		std::atomic<uint32_t> numCallbacks(0);
		pScheduler->addTask(CreateImmediateTask([&numCallbacks]() {
			return make_ready_future(5 == ++numCallbacks ? TaskResult::Break : TaskResult::Continue);
		}));

		// wait for the task to run to completion
		WAIT_FOR_VALUE(5u, numCallbacks);
		WaitForScheduled(*pScheduler, 0);

		// Assert: the task should no longer be scheduled but the callback should have been called 5 times
		EXPECT_EQ(5u, numCallbacks);
		EXPECT_EQ(0u, pScheduler->numScheduledTasks());
		EXPECT_EQ(0u, pScheduler->numExecutingTaskCallbacks());
	}

	// endregion

	// region delay timing

	namespace {
		template<typename TSleep>
		Task CreateContinuousTaskWithCounterAndSleep(
				uint32_t startDelayMs,
				uint32_t repeatDelayMs,
				std::atomic<uint32_t>& counter,
				TSleep sleep) {
			return {
				utils::TimeSpan::FromMilliseconds(startDelayMs),
				CreateUniformDelayGenerator(utils::TimeSpan::FromMilliseconds(repeatDelayMs)),
				[&counter, sleep]() {
					++counter;
					return sleep();
				},
				"task with counter"
			};
		}

		Task CreateContinuousTaskWithCounter(
				uint32_t startDelayMs,
				uint32_t repeatDelayMs,
				uint32_t callbackDelayMs,
				std::atomic<uint32_t>& counter) {
			return CreateContinuousTaskWithCounterAndSleep(startDelayMs, repeatDelayMs, counter, [callbackDelayMs]() {
				test::Sleep(callbackDelayMs);
				return make_ready_future(TaskResult::Continue);
			});
		}

		Task CreateContinuousAsyncTaskWithCounter(
				boost::asio::io_context& ioContext,
				uint32_t startDelayMs,
				uint32_t repeatDelayMs,
				uint32_t callbackDelayMs,
				std::atomic<uint32_t>& counter) {
			auto pTimer = std::make_shared<boost::asio::steady_timer>(ioContext);
			return CreateContinuousTaskWithCounterAndSleep(startDelayMs, repeatDelayMs, counter, [callbackDelayMs, pTimer]() {
				auto pPromise = std::make_shared<promise<TaskResult>>();
				pTimer->expires_from_now(std::chrono::milliseconds(callbackDelayMs));
				pTimer->async_wait([pPromise](const auto&) {
					pPromise->set_value(TaskResult::Continue);
				});

				return pPromise->get_future();
			});
		}
	}

#define EXPECT_EQ_RETRY(EXPECTED, ACTUAL) test::ExpectEqualOrRetry((EXPECTED), (ACTUAL), #EXPECTED, #ACTUAL)

	TEST(TEST_CLASS, InitialDelayIsRespected) {
		// Assert: non-deterministic because delay is impacted by scheduling
		test::RunNonDeterministicTest("Scheduler", [](auto i) {
			// Arrange: create a scheduler and add a single task to it
			auto timeUnit = test::GetTimeUnitForIteration(i);
			auto pScheduler = CreateScheduler();
			std::atomic<uint32_t> counter(0);
			pScheduler->addTask(CreateContinuousTaskWithCounter(2 * timeUnit, 20 * timeUnit, 0, counter));

			// Assert: after sleeping 0.5x the initial delay, no tasks should have run
			test::Sleep(timeUnit);
			if (!EXPECT_EQ_RETRY(0u, counter))
				return false;

			// Assert: after sleeping 1.5x the initial delay, one task should have run and
			//         the task should still be scheduled
			test::Sleep(2 * timeUnit);
			if (!EXPECT_EQ_RETRY(1u, counter))
				return false;

			EXPECT_EQ(1u, pScheduler->numScheduledTasks());
			return true;
		});
	}

	TEST(TEST_CLASS, RepeatDelayIsRespected) {
		// Assert: non-deterministic because delay is impacted by scheduling
		test::RunNonDeterministicTest("Scheduler", [](auto i) {
			// Arrange: create a scheduler and add a single task to it
			auto timeUnit = test::GetTimeUnitForIteration(i);
			auto pScheduler = CreateScheduler();
			std::atomic<uint32_t> counter(0);
			pScheduler->addTask(CreateContinuousTaskWithCounter(timeUnit, 2 * timeUnit, 0, counter));

			// Assert: after sleeping 6x, the timer should have fired at 1, 3, 5
			test::Sleep(6 * timeUnit);
			if (!EXPECT_EQ_RETRY(3u, counter))
				return false;

			EXPECT_EQ(1u, pScheduler->numScheduledTasks());
			return true;
		});
	}

	TEST(TEST_CLASS, NonConstantRepeatDelayIsRespected) {
		// Assert: non-deterministic because delay is impacted by scheduling
		test::RunNonDeterministicTest("Scheduler", [](auto i) {
			// Arrange: create a scheduler and add a single task to it
			auto timeUnit = test::GetTimeUnitForIteration(i);
			auto pScheduler = CreateScheduler();
			std::atomic<uint32_t> counter(0);

			// - configure the delays to be: 1 (start), 4, 1, 2, 10
			size_t delayCounter = 0;
			auto task = CreateContinuousTaskWithCounter(timeUnit, 4 * timeUnit, 0, counter);
			task.NextDelay = [delayCounter, timeUnit]() mutable {
				std::vector<uint32_t> delays{ 4, 1, 2, 10 };
				auto delay = delays[std::min(delayCounter++, delays.size() - 1)];
				return utils::TimeSpan::FromMilliseconds(delay * timeUnit);
			};
			pScheduler->addTask(task);

			// Assert: after sleeping 9x, the timer should have fired at 1, 5, 6, 8
			test::Sleep(9 * timeUnit);
			if (!EXPECT_EQ_RETRY(4u, counter))
				return false;

			EXPECT_EQ(1u, pScheduler->numScheduledTasks());
			return true;
		});
	}

	namespace {
		template<typename TCreateTask>
		void AssertRepeatDelayIsRelativeToCallbackTime(TCreateTask createTask) {
			// Assert: non-deterministic because delay is impacted by scheduling
			test::RunNonDeterministicTest("Scheduler", [createTask](auto i) {
				// Arrange: create a scheduler and add a single task to it
				auto timeUnit = test::GetTimeUnitForIteration(i);
				auto pScheduler = CreateScheduler();
				std::atomic<uint32_t> counter(0);
				pScheduler->addTask(createTask(0u, 2u * timeUnit, 3u * timeUnit, counter));

				// Assert: after sleeping 6x, the timer should have fired at 0, 5
				test::Sleep(6 * timeUnit);
				if (!EXPECT_EQ_RETRY(2u, counter))
					return false;

				EXPECT_EQ(1u, pScheduler->numScheduledTasks());
				return true;
			});
		}
	}

	TEST(TEST_CLASS, RepeatDelayIsRelativeToCallbackTime_Blocking) {
		AssertRepeatDelayIsRelativeToCallbackTime(CreateContinuousTaskWithCounter);
	}

	TEST(TEST_CLASS, RepeatDelayIsRelativeToCallbackTime_NonBlocking) {
		// Arrange: create pool here so that current thread joins the pool (in the pool destructor)
		auto pPool = test::CreateStartedIoThreadPool(1);

		// Assert:
		AssertRepeatDelayIsRelativeToCallbackTime([&pPool](auto startDelayMs, auto repeatDelayMs, auto callbackDelayMs, auto& counter) {
			return CreateContinuousAsyncTaskWithCounter(pPool->ioContext(), startDelayMs, repeatDelayMs, callbackDelayMs, counter);
		});
	}

#undef EXPECT_EQ_RETRY

	// endregion
}}
