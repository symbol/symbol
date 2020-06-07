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

#include "Scheduler.h"
#include "FutureUtils.h"
#include "IoThreadPool.h"
#include "StrandOwnerLifetimeExtender.h"
#include "catapult/utils/Logging.h"
#include "catapult/utils/WeakContainer.h"
#include "catapult/exceptions.h"
#include "catapult/preprocessor.h"
#include <boost/asio/steady_timer.hpp>
#include <boost/asio.hpp>

namespace catapult { namespace thread {

	namespace {
		std::chrono::milliseconds ToMillis(const utils::TimeSpan& timeSpan) {
			return std::chrono::milliseconds(timeSpan.millis());
		}

		// wraps a task using an implicit strand
		template<typename TCallbackWrapper>
		class BasicTaskWrapper {
		public:
			BasicTaskWrapper(boost::asio::io_context& ioContext, const Task& task, TCallbackWrapper& wrapper)
					: m_task(task)
					, m_wrapper(wrapper)
					, m_timer(ioContext, ToMillis(task.StartDelay))
					, m_isStopped(false) {
				CATAPULT_LOG(debug) << "task '" << m_task.Name << "' is scheduled in " << task.StartDelay;
			}

		public:
			void start() {
				startWait();
			}

			void stop() {
				m_isStopped = true;
				m_timer.cancel();
			}

		private:
			void startWait() {
				if (m_isStopped) {
					CATAPULT_LOG(trace) << "bypassing start of stopped timer";
					return;
				}

				m_timer.async_wait(m_wrapper.wrap([this](const auto& ec) { this->handleWait(ec); }));
			}

			void handleWait(const boost::system::error_code& ec) {
				if (ec) {
					if (boost::asio::error::operation_aborted == ec)
						return;

					CATAPULT_THROW_EXCEPTION(boost::system::system_error(ec));
				}

				m_task.Callback().then(m_wrapper.wrapFutureContinuation([this](auto result) { this->handleCompletion(result); }));
			}

			void handleCompletion(TaskResult result) {
				if (result == TaskResult::Break) {
					CATAPULT_LOG(warning) << "task '" << m_task.Name << "' broke and will be stopped";
					return;
				}

				auto nextDelay = m_task.NextDelay();
				CATAPULT_LOG(trace) << "task '" << m_task.Name << "' will continue in " << nextDelay;
				m_timer.expires_from_now(ToMillis(nextDelay));
				startWait();
			}

		private:
			Task m_task;
			TCallbackWrapper& m_wrapper;
			boost::asio::steady_timer m_timer;
			bool m_isStopped;
		};

		// wraps a task using using an explicit strand and ensures deterministic shutdown by using enable_shared_from_this
		class StrandedTaskWrapper : public std::enable_shared_from_this<StrandedTaskWrapper> {
		public:
			StrandedTaskWrapper(boost::asio::io_context& ioContext, const Task& task)
					: m_strand(ioContext)
					, m_strandWrapper(m_strand)
					, m_impl(ioContext, task, *this)
			{}

		public:
			void start() {
				post([](auto& impl) { impl.start(); });
			}

			void stop() {
				post([](auto& impl) { impl.stop(); });
			}

		public:
			template<typename THandler>
			auto wrap(THandler handler) {
				return m_strandWrapper.wrap(shared_from_this(), handler);
			}

			template<typename THandler>
			auto wrapFutureContinuation(THandler handler) {
				// boost asio callbacks need to be copyable, so they do not support move-only arguments (e.g. future)
				// as a workaround, call future.get() outside of the strand and post the result onto the strand
				return [pThis = shared_from_this(), handler](auto&& future) {
					return pThis->m_strandWrapper.wrap(pThis, handler)(future.get());
				};
			}

		private:
			template<typename THandler>
			void post(THandler handler) {
				return m_strandWrapper.post(shared_from_this(), [handler](const auto& pThis) {
					handler(pThis->m_impl);
				});
			}

		private:
			boost::asio::io_context::strand m_strand;
			StrandOwnerLifetimeExtender<StrandedTaskWrapper> m_strandWrapper;
			BasicTaskWrapper<StrandedTaskWrapper> m_impl;
		};

		class DefaultScheduler
				: public Scheduler
				, public std::enable_shared_from_this<DefaultScheduler> {
		public:
			explicit DefaultScheduler(IoThreadPool& pool)
					: m_ioContext(pool.ioContext())
					, m_numExecutingTaskCallbacks(0)
					, m_isStopped(false)
					, m_tasks([](auto& task) { task.stop(); })
			{}

			~DefaultScheduler() override {
				shutdown();
			}

		public:
			uint32_t numScheduledTasks() const override {
				return static_cast<uint32_t>(m_tasks.size());
			}

			uint32_t numExecutingTaskCallbacks() const override {
				return m_numExecutingTaskCallbacks;
			}

		public:
			void addTask(const Task& task) override {
				if (m_isStopped)
					CATAPULT_THROW_RUNTIME_ERROR("cannot add new scheduled task because scheduler has shutdown");

				// wrap the task callback to automatically update m_numExecutingTaskCallbacks
				auto taskCopy = task;
				taskCopy.Callback = [pThis = shared_from_this(), callback = task.Callback]() {
					++pThis->m_numExecutingTaskCallbacks;
					return compose(callback(), [pThis](auto&& resultFuture) {
						--pThis->m_numExecutingTaskCallbacks;
						return std::move(resultFuture);
					});
				};

				auto pTask = std::make_shared<StrandedTaskWrapper>(m_ioContext, taskCopy);
				m_tasks.insert(pTask);
				pTask->start();
			}

			void shutdown() override {
				bool expectedIsStopped = false;
				if (!m_isStopped.compare_exchange_strong(expectedIsStopped, true))
					return;

				CATAPULT_LOG(trace) << "Scheduler stopping";
				m_tasks.clear();
				CATAPULT_LOG(info) << "Scheduler stopped";
			}

		private:
			boost::asio::io_context& m_ioContext;

			std::atomic<uint32_t> m_numExecutingTaskCallbacks;
			std::atomic_bool m_isStopped;
			utils::WeakContainer<StrandedTaskWrapper> m_tasks;
		};
	}

	std::shared_ptr<Scheduler> CreateScheduler(IoThreadPool& pool) {
		auto pScheduler = std::make_shared<DefaultScheduler>(pool);
		return PORTABLE_MOVE(pScheduler);
	}
}}
