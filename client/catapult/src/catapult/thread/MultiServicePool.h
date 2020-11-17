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

#pragma once
#include "IoThreadPool.h"
#include "catapult/utils/Logging.h"
#include "catapult/functions.h"
#include "catapult/preprocessor.h"
#include <atomic>
#include <memory>
#include <thread>
#include <vector>

namespace catapult { namespace thread {

	/// Manages a primary thread pool with dependent service groups and secondary isolated thread pools.
	/// \note Services are shutdown in reverse order of registration.
	class MultiServicePool {
	public:
		/// Isolated pool mode.
		enum class IsolatedPoolMode {
			/// Sub pool isolation is enabled.
			Enabled,

			/// Sub pool isolation is disabled.
			Disabled
		};

		/// Default pool concurrency level based on the local hardware configuration.
		static constexpr size_t DefaultPoolConcurrency() {
			return 0;
		}

	public:
		// region ServiceGroup

		/// Group of services that should be shutdown together and might be interdependent.
		/// \note AsyncTcpServer and accept handlers (PacketReaders, PacketWriters) have such a shutdown requirement
		///       because AsyncTcpServer includes a shared_ptr to itself as part of its accepted PacketSocket,
		///       which is then kept alive by the accept handlers until they are also shutdown.
		class ServiceGroup {
		public:
			/// Creates a group around \a pool and \a name.
			ServiceGroup(thread::IoThreadPool& pool, const std::string& name)
					: m_pool(pool)
					, m_name(name)
			{}

		public:
			/// Gets the number of services.
			size_t numServices() const {
				return m_shutdownFunctions.size();
			}

		public:
			/// Registers an externally created service (\a pService) for cleanup.
			template<typename TService>
			auto registerService(const std::shared_ptr<TService>& pService) {
				m_services.push_back(pService);

				auto serviceName = m_name + " service " + std::to_string(m_shutdownFunctions.size() + 1);
				m_shutdownFunctions.push_back([pService, serviceName]() {
					// shutdown the service
					CATAPULT_LOG(info) << "shutting down " << serviceName;
					pService->shutdown();
				});

				CATAPULT_LOG(debug) << "registered " << serviceName;
				return pService;
			}

			/// Creates a new service by calling \a factory with \a args service arguments.
			template<typename TFactory, typename... TArgs>
			auto pushService(TFactory factory, TArgs&&... args) {
				// create a service around the pool
				auto pService = factory(m_pool, std::forward<TArgs>(args)...);
				return registerService(pService);
			}

			/// Safely shuts down the service group.
			void shutdown() {
				// 1. shutdown the services
				for (auto iter = m_shutdownFunctions.rbegin(); m_shutdownFunctions.rend() != iter; ++iter)
					(*iter)();

				m_shutdownFunctions.clear();

				// 2. wait for all service references to be released
				for (const auto& pService : m_services)
					WaitForLastReference(pService);
			}

		private:
			thread::IoThreadPool& m_pool;
			std::string m_name;
			std::vector<std::shared_ptr<void>> m_services;
			std::vector<action> m_shutdownFunctions;
		};

		// endregion

	public:
		/// Creates a pool with the specified number of threads (\a numWorkerThreads) and \a name with optional
		/// isolated pool mode (\a isolatedPoolMode).
		/// \note If \a numWorkerThreads is \c 0, a default number of threads will be used.
		MultiServicePool(const std::string& name, size_t numWorkerThreads, IsolatedPoolMode isolatedPoolMode = IsolatedPoolMode::Enabled)
				: m_name(name)
				, m_isolatedPoolMode(isolatedPoolMode)
				, m_numTotalIsolatedPoolThreads(0)
				, m_numServiceGroups(0)
				, m_pPool(CreateThreadPool(numWorkerThreads, name))
		{}

		/// Destroys the pool.
		~MultiServicePool() {
			shutdown();
		}

	public:
		/// Gets the number of active worker threads.
		size_t numWorkerThreads() const {
			return (m_pPool ? m_pPool->numWorkerThreads() : 0) + m_numTotalIsolatedPoolThreads;
		}

		/// Gets the number of service groups.
		size_t numServiceGroups() const {
			return m_numServiceGroups;
		}

		/// Gets the number of services.
		/// \note This accessor is NOT threadsafe.
		size_t numServices() const {
			// don't include the service groups in the count
			auto numServices = m_shutdownFunctions.size() - numServiceGroups();
			for (const auto& pGroup : m_serviceGroups)
				numServices += pGroup->numServices();

			return numServices;
		}

	private:
		template<typename TService>
		auto registerService(const std::shared_ptr<TService>& pService, const std::string& serviceName) {
			m_shutdownFunctions.push_back([pService, serviceName]() {
				CATAPULT_LOG(info) << "shutting down " << serviceName;

				// shutdown the service and wait for all callbacks to complete
				pService->shutdown();
				WaitForLastReference(pService);
			});

			return pService;
		}

	public:
		/// Creates a new service group with \a name.
		std::shared_ptr<ServiceGroup> pushServiceGroup(const std::string& name) {
			auto pGroup = std::make_shared<ServiceGroup>(*m_pPool, m_name + "::" + name);
			m_serviceGroups.push_back(pGroup);
			registerService(pGroup, name + " (service group)");
			++m_numServiceGroups;
			return pGroup;
		}

		/// Creates a new isolated thread pool with a default number of threads and \a name.
		thread::IoThreadPool* pushIsolatedPool(const std::string& name) {
			return pushIsolatedPool(name, DefaultPoolConcurrency());
		}

		/// Creates a new isolated thread pool with the specified number of threads (\a numWorkerThreads) and \a name.
		/// \note If \a numWorkerThreads is \c 0, a default number of threads will be used.
		thread::IoThreadPool* pushIsolatedPool(const std::string& name, size_t numWorkerThreads) {
			class PoolServiceAdapter {
			public:
				explicit PoolServiceAdapter(std::unique_ptr<thread::IoThreadPool>&& pPool) : m_pPool(std::move(pPool))
				{}

			public:
				void shutdown() {
					DestroyThreadPool(m_pPool);
				}

			private:
				std::unique_ptr<thread::IoThreadPool> m_pPool;
			};

			// when isolated pool mode is disabled, use the main pool for everything
			if (IsolatedPoolMode::Disabled == m_isolatedPoolMode)
				return m_pPool.get();

			auto pPool = CreateThreadPool(numWorkerThreads, name);
			auto* pPoolRaw = pPool.get();

			registerService(std::make_shared<PoolServiceAdapter>(std::move(pPool)), name + " (isolated pool)");

			m_numTotalIsolatedPoolThreads += pPoolRaw->numWorkerThreads();
			return pPoolRaw;
		}

	public:
		/// Safely shuts down the thread pool and its dependent services.
		void shutdown() {
			// if the pool has already been destroyed, don't do anything
			if (!m_pPool)
				return;

			// 1. clear the dependent entities
			m_serviceGroups.clear();

			// 2. shutdown the services
			for (auto iter = m_shutdownFunctions.rbegin(); m_shutdownFunctions.rend() != iter; ++iter)
				(*iter)();

			m_shutdownFunctions.clear();
			m_numTotalIsolatedPoolThreads = 0;
			m_numServiceGroups = 0;

			// 3. after the services are destroyed, the thread pool can be safely destroyed
			DestroyThreadPool(m_pPool);
		}

	private:
		static std::unique_ptr<thread::IoThreadPool> CreateThreadPool(size_t numWorkerThreads, const std::string& name) {
			numWorkerThreads = DefaultPoolConcurrency() == numWorkerThreads ? std::thread::hardware_concurrency() : numWorkerThreads;
			auto pPool = thread::CreateIoThreadPool(numWorkerThreads, name.c_str());
			pPool->start();
			return pPool;
		}

		static void DestroyThreadPool(std::unique_ptr<thread::IoThreadPool>& pPool) {
			pPool->join();
			pPool.reset();
		}

		template<typename T>
		static void WaitForLastReference(const std::shared_ptr<T>& pVoid) {
			volatile long useCount;
			while (1 < (useCount = pVoid.use_count()))
				std::this_thread::yield();
		}

	private:
		std::string m_name;
		IsolatedPoolMode m_isolatedPoolMode;
		std::atomic<size_t> m_numTotalIsolatedPoolThreads;
		std::atomic<size_t> m_numServiceGroups;
		std::unique_ptr<thread::IoThreadPool> m_pPool;
		std::vector<std::shared_ptr<ServiceGroup>> m_serviceGroups;
		std::vector<action> m_shutdownFunctions;
	};
}}
