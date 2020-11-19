/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/thread/IoThreadPool.h"
#include "catapult/ionet/IoTypes.h"
#include "catapult/utils/AtomicIncrementDecrementGuard.h"
#include "tests/test/core/WaitFunctions.h"
#include "tests/TestHarness.h"
#include <memory>
#include <thread>

namespace catapult { namespace thread {

#define TEST_CLASS IoThreadPoolTests

	namespace {
		const uint32_t Num_Default_Threads = test::GetNumDefaultPoolThreads();

		auto CreateDefaultIoThreadPool() {
			return CreateIoThreadPool(Num_Default_Threads);
		}
	}

	TEST(TEST_CLASS, CanCreateThreadPoolWithDefaultName) {
		// Act: set up a pool with a default name
		auto pPool = CreateDefaultIoThreadPool();

		// Assert:
		EXPECT_EQ("", pPool->name());
	}

	TEST(TEST_CLASS, CanCreateThreadPoolWithCustomName) {
		// Act: set up a pool with a custom name
		auto pPool = CreateIoThreadPool(Num_Default_Threads, "Crazy Amazing");

		// Assert:
		EXPECT_EQ("Crazy Amazing", pPool->name());
	}

	TEST(TEST_CLASS, ConstructorDoesNotCreateAnyThreads) {
		// Act: set up a pool
		auto pPool = CreateDefaultIoThreadPool();

		// Assert:
		EXPECT_EQ(0u, pPool->numWorkerThreads());
	}

	TEST(TEST_CLASS, StartSpawnsSpecifiedNumberOfWorkerThreads) {
		// Act: set up a pool
		auto pPool = CreateDefaultIoThreadPool();
		pPool->start();

		// Assert: all threads have been spawned
		EXPECT_EQ(Num_Default_Threads, pPool->numWorkerThreads());
	}

	TEST(TEST_CLASS, JoinDestroysAllWorkerThreads) {
		// Arrange: set up a pool
		auto pPool = CreateDefaultIoThreadPool();
		pPool->start();

		// Act: stop the pool
		pPool->join();

		// Assert: all threads have been stopped
		EXPECT_EQ(0u, pPool->numWorkerThreads());
	}

	TEST(TEST_CLASS, JoinIsIdempotent) {
		// Arrange: set up a pool
		auto pPool = CreateDefaultIoThreadPool();
		pPool->start();

		// Act: stop the pool
		for (auto i = 0; i < 3; ++i)
			pPool->join();

		// Assert: all threads have been stopped
		EXPECT_EQ(0u, pPool->numWorkerThreads());
	}

	TEST(TEST_CLASS, PoolCanBeRestarted) {
		// Arrange: set up a pool
		auto pPool = CreateDefaultIoThreadPool();
		pPool->start();

		// Act: restart the pool
		pPool->join();
		pPool->start();

		// Assert: all threads have been spawned
		EXPECT_EQ(Num_Default_Threads, pPool->numWorkerThreads());
	}

	TEST(TEST_CLASS, PoolCannotBeRestartedWhenRunning) {
		// Arrange: set up a pool
		auto pPool = CreateDefaultIoThreadPool();
		pPool->start();

		// Act + Assert: restart the pool
		EXPECT_THROW(pPool->start(), catapult_runtime_error);
	}

	TEST(TEST_CLASS, JoinDoesNotAbortThreads) {
		// Arrange: set up a pool
		auto pPool = CreateDefaultIoThreadPool();
		pPool->start();

		// - post some work on it
		std::atomic<uint32_t> numWaits(0);
		std::atomic<uint32_t> maxWaits(10000);
		std::atomic_bool isHandlerExecuting(false);
		boost::asio::post(pPool->ioContext(), [&]() {
			isHandlerExecuting = true;
			while (numWaits < maxWaits) {
				test::Sleep(1);
				++numWaits;
			}
		});
		WAIT_FOR(isHandlerExecuting);

		// Act: stop the pool
		uint32_t preShutdownWaits = numWaits;
		maxWaits = numWaits + 10u;
		pPool->join();

		// Assert: the posted work was allowed to complete and was not aborted
		CATAPULT_LOG(debug)
			<< "preShutdownWaits " << preShutdownWaits
			<< " numWaits " << numWaits
			<< " maxWaits " << maxWaits;
		EXPECT_LE(10u, maxWaits - preShutdownWaits);
		EXPECT_EQ(maxWaits, numWaits);
	}

	TEST(TEST_CLASS, PoolCanServeMoreRequestsThanWorkerThreads) {
		// Arrange: set up a pool
		auto pPool = CreateDefaultIoThreadPool();
		pPool->start();

		// - post 100 work items on the pool
		std::atomic<uint32_t> numHandlerCalls(0);
		for (auto i = 0u; i < 100; ++i)
			boost::asio::post(pPool->ioContext(), [&]() { ++numHandlerCalls; });

		// Act: stop the pool
		pPool->join();

		// Assert: the pool should have executed 100 work items
		EXPECT_EQ(100u, numHandlerCalls);
	}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

	TEST(TEST_CLASS, PoolFailsFastWhenAcceptHandlerExcepts) {
		// Assert: if an exception bubbles out of thread pool work, program termination is expected
		ASSERT_DEATH([]() {
			// Arrange: set up a pool
			auto pPool = CreateDefaultIoThreadPool();
			pPool->start();

			// - cause one work item to except
			std::atomic<uint32_t> numHandlerCalls(0);
			for (auto i = 0u; i < 10; ++i) {
				boost::asio::post(pPool->ioContext(), [&]() {
					if (7u == ++numHandlerCalls)
						CATAPULT_THROW_RUNTIME_ERROR("exception from thread pool thread");
				});
			}

			// Act: wait for the pool
			pPool->join();
		}(), "");
	}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

	namespace {
		constexpr uint32_t Wait_Duration_Millis = 5;

		class AbstractBlockingWork {
		public:
			AbstractBlockingWork(IoThreadPool& pool, const test::WaitFunction& wait)
					: m_pool(pool)
					, m_wait(wait)
					, m_shouldWait(true)
					, m_numHandlerCalls(0)
			{}

			virtual ~AbstractBlockingWork() {
				stop();
			}

		public:
			uint32_t numHandlerCalls() const {
				return m_numHandlerCalls;
			}

		public:
			void post(size_t numPosts) {
				for (auto i = 0u; i < numPosts; ++i)
					postOne();
			}

			void postOne() {
				boost::asio::post(m_pool.ioContext(), [this]() {
					++m_numHandlerCalls;
					this->m_wait(m_pool.ioContext(), [this]() { return m_shouldWait.load(); });
				});
			}

			void waitForNumHandlerCalls(uint32_t numCalls) {
				WAIT_FOR_VALUE(numCalls, m_numHandlerCalls);
			}

		private:
			void stop() {
				CATAPULT_LOG(debug) << "test completed - joining all threads";
				m_shouldWait = false;
				m_pool.join();
			}

		private:
			IoThreadPool& m_pool;
			test::WaitFunction m_wait;
			std::atomic_bool m_shouldWait;
			std::atomic<uint32_t> m_numHandlerCalls;
		};

		class BlockingWork : public AbstractBlockingWork {
		public:
			explicit BlockingWork(IoThreadPool& pool)
					: AbstractBlockingWork(pool, test::CreateSyncWaitFunction(Wait_Duration_Millis))
			{}
		};

		class NonBlockingWork : public AbstractBlockingWork {
		public:
			explicit NonBlockingWork(IoThreadPool& pool)
					: AbstractBlockingWork(pool, test::CreateAsyncWaitFunction(Wait_Duration_Millis))
			{}
		};
	}

	TEST(TEST_CLASS, PoolWorkerThreadsCannotServiceAdditionalRequestsWhenHandlersWaitBlocking) {
		// Arrange: set up a pool
		auto pPool = CreateDefaultIoThreadPool();

		// - post 2X work items on the pool (blocking)
		BlockingWork work(*pPool);
		work.post(2 * Num_Default_Threads);
		pPool->start();

		// Act: wait for X threads to enter the handler
		work.waitForNumHandlerCalls(Num_Default_Threads);

		// - wait a bit to give the pool time to handle more work if there is a bug in the implementation
		test::Pause();

		// Assert: only X / 2X work items were processed
		EXPECT_EQ(Num_Default_Threads, pPool->numWorkerThreads());
		EXPECT_EQ(Num_Default_Threads, work.numHandlerCalls());
	}

	TEST(TEST_CLASS, PoolWorkerThreadsCanServiceAdditionalRequestsWhenHandlersWaitNonBlocking) {
		// Arrange: set up a pool
		auto pPool = CreateDefaultIoThreadPool();

		// - post 2X work items on the pool (non blocking)
		NonBlockingWork work(*pPool);
		work.post(2 * Num_Default_Threads);
		pPool->start();

		// Act: wait for 2X threads to enter the handler
		work.waitForNumHandlerCalls(2 * Num_Default_Threads);

		// - wait a bit to give the pool time to handle more work if there is a bug in the implementation
		test::Pause();

		// Assert: all work items were processed
		EXPECT_EQ(Num_Default_Threads, pPool->numWorkerThreads());
		EXPECT_EQ(2 * Num_Default_Threads, work.numHandlerCalls());
	}
}}
