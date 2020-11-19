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

#include "IoThreadPool.h"
#include "ThreadGroup.h"
#include "ThreadInfo.h"
#include "catapult/utils/AtomicIncrementDecrementGuard.h"
#include "catapult/utils/Logging.h"
#include "catapult/exceptions.h"
#include <boost/asio.hpp>

namespace catapult { namespace thread {

	namespace {
		// helper RAII class to simplify a restartable thread pool with limitless work
		class ThreadPoolContext {
		public:
			explicit ThreadPoolContext(boost::asio::io_context& ioContext) : m_work(boost::asio::make_work_guard(ioContext)) {
				ioContext.reset();
			}

			~ThreadPoolContext() {
				// destroy the work before waiting for the thread pool threads to stop
				m_work.reset();
				m_threads.join();
			}

		public:
			inline size_t numThreads() const {
				return m_threads.size();
			}

			template<typename TThreadFunc>
			inline void createThread(TThreadFunc threadFunc) {
				m_threads.spawn(threadFunc);
			}

		private:
			boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_work;
			ThreadGroup m_threads;
		};

		class DefaultIoThreadPool : public IoThreadPool {
		public:
			DefaultIoThreadPool(size_t numWorkerThreads, const std::string& name)
					: m_numConfiguredWorkerThreads(numWorkerThreads)
					, m_name(name)
					, m_tag(m_name.empty() ? std::string() : " (" + m_name + ")")
					, m_numWorkerThreads(0)
			{}

			~DefaultIoThreadPool() override {
				join();
			}

		public:
			uint32_t numWorkerThreads() const override {
				return m_numWorkerThreads;
			}

			const std::string& name() const override {
				return m_name;
			}

			boost::asio::io_context& ioContext() override {
				return m_ioContext;
			}

		public:
			void start() override {
				if (0 != m_numWorkerThreads)
					CATAPULT_THROW_RUNTIME_ERROR_1("cannot restart running thread pool", m_numWorkerThreads);

				// spawn the number of configured threads
				CATAPULT_LOG(trace) << "spawning threads" << m_tag;
				m_pContext = std::make_unique<ThreadPoolContext>(m_ioContext);
				for (auto i = 0u; i < m_numConfiguredWorkerThreads; ++i) {
					m_pContext->createThread([this, i]() {
						thread::SetThreadName(std::to_string(i) + this->m_tag + " worker");
						ioWorkerFunction();
					});
				}

				// wait for the threads to be spawned
				CATAPULT_LOG(trace) << "waiting for threads to be spawned" << m_tag;
				while (m_numWorkerThreads < m_numConfiguredWorkerThreads) {}
				CATAPULT_LOG(info) << "spawned " << m_pContext->numThreads() << " workers" << m_tag;
			}

			void join() override {
				if (!m_pContext)
					return;

				CATAPULT_LOG(debug) << "waiting for " << m_numWorkerThreads << " thread pool threads to exit" << m_tag;
				m_pContext.reset();
				CATAPULT_LOG(info) << "all thread pool threads exited" << m_tag;
			}

		private:
			void ioWorkerFunction() {
				CATAPULT_LOG(trace) << "worker thread started" << m_tag;

				auto incrementDecrementGuard = utils::MakeIncrementDecrementGuard(m_numWorkerThreads);
				m_ioContext.run();

				CATAPULT_LOG(trace) << "worker thread finished" << m_tag;
			}

		private:
			size_t m_numConfiguredWorkerThreads;
			std::string m_name;
			std::string m_tag;

			boost::asio::io_context m_ioContext;
			std::unique_ptr<ThreadPoolContext> m_pContext;
			std::atomic<uint32_t> m_numWorkerThreads;
		};
	}

	std::unique_ptr<IoThreadPool> CreateIoThreadPool(size_t numWorkerThreads, const char* name) {
		return std::make_unique<DefaultIoThreadPool>(numWorkerThreads, name ? std::string(name) : std::string());
	}
}}
