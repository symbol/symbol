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

#include "RootedService.h"
#include "ServiceLocator.h"

namespace catapult { namespace extensions {

	namespace {
		class RootedServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			RootedServiceRegistrar(const std::shared_ptr<void>& pService, const std::string& serviceName, ServiceRegistrarPhase phase)
					: m_pService(pService)
					, m_serviceName(serviceName)
					, m_phase(phase)
			{}

		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "Rooted - " + m_serviceName, m_phase };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no additional counters
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState&) override {
				// register rooted service
				locator.registerRootedService(m_serviceName, m_pService);
			}

		private:
			std::shared_ptr<void> m_pService;
			std::string m_serviceName;
			ServiceRegistrarPhase m_phase;
		};
	}

	DECLARE_SERVICE_REGISTRAR(Rooted)(const std::shared_ptr<void>& pService, const std::string& serviceName, ServiceRegistrarPhase phase) {
		return std::make_unique<RootedServiceRegistrar>(pService, serviceName, phase);
	}
}}
