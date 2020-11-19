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

#pragma once
#include "catapult/utils/DiagnosticCounter.h"
#include "catapult/exceptions.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace catapult { namespace config { class CatapultKeys; } }

namespace catapult { namespace extensions {

	/// Service locator for local node services.
	class ServiceLocator {
	public:
		/// Value that is returned when a statistics source is \c nullptr.
		static constexpr uint64_t Sentinel_Counter_Value = static_cast<uint64_t>(-1);

	public:
		/// Creates a locator around \a keys.
		explicit ServiceLocator(const config::CatapultKeys& keys) : m_keys(keys)
		{}

		/// Destroys the locator.
		~ServiceLocator() {
			CATAPULT_LOG(info) << "destroying " << m_rootedServices.size() << " rooted services";

			// destroy rooted services in reverse order of registration
			auto i = 1u;
			while (!m_rootedServices.empty()) {
				CATAPULT_LOG(debug) << i << ": destroying rooted service " << m_rootedServices.back().first;
				m_rootedServices.pop_back();
				++i;
			}
		}

	public:
		/// Gets the local keys.
		const config::CatapultKeys& keys() const {
			return m_keys;
		}

		/// Gets the diagnostic counters.
		const std::vector<utils::DiagnosticCounter>& counters() const {
			return m_counters;
		}

		/// Gets the number of registered services.
		size_t numServices() const {
			return m_services.size();
		}

		/// Gets the service with \a serviceName.
		template<typename TService>
		std::shared_ptr<TService> service(const std::string& serviceName) const {
			std::shared_ptr<TService> pService;
			if (!tryGetService(serviceName, pService))
				CATAPULT_THROW_INVALID_ARGUMENT_1("requested service is not registered", serviceName);

			return pService;
		}

	public:
		/// Adds a service (\a pService) with \a serviceName.
		void registerService(const std::string& serviceName, const std::shared_ptr<void>& pService) {
			if (!m_services.emplace(serviceName, pService).second)
				CATAPULT_THROW_INVALID_ARGUMENT_1("service is already registered", serviceName);
		}

		/// Adds a rooted service (\a pService) with \a serviceName that is kept alive by the service locator.
		void registerRootedService(const std::string& serviceName, const std::shared_ptr<void>& pService) {
			registerService(serviceName, pService);
			m_rootedServices.emplace_back(serviceName, pService);
		}

		/// Adds a service-dependent counter with \a counterName for service \a serviceName given \a supplier.
		template<typename TService, typename TSupplier>
		void registerServiceCounter(const std::string& serviceName, const std::string& counterName, TSupplier supplier) {
			m_counters.emplace_back(utils::DiagnosticCounterId(counterName), [this, serviceName, supplier]() {
				std::shared_ptr<TService> pService;
				this->tryGetService(serviceName, pService);
				return pService ? supplier(*pService) : Sentinel_Counter_Value;
			});
		}

	private:
		template<typename TService>
		bool tryGetService(const std::string& serviceName, std::shared_ptr<TService>& pService) const {
			auto iter = m_services.find(serviceName);
			if (m_services.cend() == iter)
				return false;

			auto pVoidService = iter->second.lock();
			pService = std::shared_ptr<TService>(pVoidService, reinterpret_cast<TService*>(pVoidService.get()));
			return true;
		}

	private:
		const config::CatapultKeys& m_keys;
		std::vector<utils::DiagnosticCounter> m_counters;
		std::unordered_map<std::string, std::weak_ptr<void>> m_services;
		std::vector<std::pair<std::string, std::shared_ptr<void>>> m_rootedServices;
	};
}}
