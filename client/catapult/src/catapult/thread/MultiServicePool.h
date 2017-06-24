#pragma once
#include "IoServiceThreadPool.h"
#include "catapult/utils/Logging.h"
#include <memory>
#include <thread>
#include <vector>

namespace catapult { namespace thread {

	/// Manages a primary threadpool with dependent service groups and secondary isolated threadpools.
	/// \note Services are shutdown in reverse order of registration.
	class MultiServicePool {
	public:
		/// A default pool concurrency level based on the local hardware configuration.
		static constexpr size_t DefaultPoolConcurrency() {
			return 0;
		}

	public:
		// region ServiceGroup

		/// A group of services that should be shutdown together and might be interdependent.
		/// \note AsyncTcpServer and accept handlers (PacketReaders, PacketWriters) have such a shutdown requirement
		///       because AsyncTcpServer includes a shared_ptr to itself as part of its AsyncTcpServerAcceptContext,
		///       which is then kept alive by the accept handlers until they are also shutdown.
		class ServiceGroup {
		public:
			/// Creates a group around \a pPool and \a name.
			ServiceGroup(const std::shared_ptr<thread::IoServiceThreadPool>& pPool, const std::string& name)
					: m_pPool(pPool)
					, m_name(name)
			{}

		public:
			/// The number of services.
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

				return pService;
			}

			/// Creates a new service by calling \a factory with \a args service arguments.
			template<typename TFactory, typename... TArgs>
			auto pushService(TFactory factory, TArgs&&... args) {
				// create a service around the pool
				auto pService = factory(m_pPool, std::forward<TArgs>(args)...);
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
			std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
			std::string m_name;
			std::vector<std::shared_ptr<void>> m_services;
			std::vector<std::function<void ()>> m_shutdownFunctions;
		};

		// endregion

	public:
		/// Creates a pool with the specified number of threads (\a numWorkerThreads) and \a name.
		/// \note If \a numWorkerThreads is \c 0, a default number of threads will be used.
		MultiServicePool(size_t numWorkerThreads, const std::string& name)
				: m_name(name)
				, m_numTotalIsolatedPoolThreads(0)
				, m_numServiceGroups(0)
				, m_pPool(CreateThreadPool(numWorkerThreads, name))
		{}

		/// Destroys the pool.
		~MultiServicePool() {
			shutdown();
		}

	public:
		/// The number of active worker threads.
		size_t numWorkerThreads() const {
			return (m_pPool ? m_pPool->numWorkerThreads() : 0) + m_numTotalIsolatedPoolThreads;
		}

		/// The number of service groups.
		size_t numServiceGroups() const {
			return m_numServiceGroups;
		}

		/// The number of services.
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
			auto pGroup = std::make_shared<ServiceGroup>(m_pPool, m_name + "::" + name);
			m_serviceGroups.push_back(pGroup);
			registerService(pGroup, name + " (service group)");
			++m_numServiceGroups;
			return pGroup;
		}

		/// Creates a new isolated threadpool with the specified number of threads (\a numWorkerThreads) and \a name.
		/// \note If \a numWorkerThreads is \c 0, a default number of threads will be used.
		std::shared_ptr<thread::IoServiceThreadPool> pushIsolatedPool(size_t numWorkerThreads, const std::string& name) {
			class PoolServiceAdapter {
			public:
				explicit PoolServiceAdapter(const std::shared_ptr<thread::IoServiceThreadPool>& pPool) : m_pPool(pPool)
				{}

			public:
				void shutdown() {
					WaitForLastReference(m_pPool);
					m_pPool->join();
				}

			private:
				std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
			};

			auto pPool = CreateThreadPool(numWorkerThreads, name);
			registerService(std::make_shared<PoolServiceAdapter>(pPool), name + " (isolated pool)");
			m_numTotalIsolatedPoolThreads += pPool->numWorkerThreads();
			return pPool;
		}

	public:
		/// Safely shuts down the threadpool and its dependent services.
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

			// 3. after the services are destroyed, the threadpool can be safely destroyed
			WaitForLastReference(m_pPool);
			m_pPool.reset();
		}

	private:
		static std::shared_ptr<thread::IoServiceThreadPool> CreateThreadPool(size_t numWorkerThreads, const std::string& name) {
			numWorkerThreads = DefaultPoolConcurrency() == numWorkerThreads ? std::thread::hardware_concurrency() : numWorkerThreads;
			auto pPool = thread::CreateIoServiceThreadPool(numWorkerThreads, name.c_str());
			pPool->start();
			return std::move(pPool);
		}

		template<typename T>
		static void WaitForLastReference(const std::shared_ptr<T>& pVoid) {
			volatile bool isLastReference;
			while (false == (isLastReference = pVoid.unique()))
				std::this_thread::yield();
		}

	private:
		std::string m_name;
		size_t m_numTotalIsolatedPoolThreads;
		size_t m_numServiceGroups;
		std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
		std::vector<std::shared_ptr<ServiceGroup>> m_serviceGroups;
		std::vector<std::function<void ()>> m_shutdownFunctions;
	};
}}
