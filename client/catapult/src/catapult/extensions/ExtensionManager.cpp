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

#include "ExtensionManager.h"
#include "BasicServerHooks.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/NetworkTime.h"

namespace catapult { namespace extensions {

	ExtensionManager::ExtensionManager() {
		for (const auto& pluginName : { "catapult.plugins.coresystem", "catapult.plugins.signature" })
			m_systemPluginNames.push_back(pluginName);
	}

	void ExtensionManager::registerSystemPlugin(const std::string& name) {
		m_systemPluginNames.push_back(name);
	}

	void ExtensionManager::setNetworkTimeSupplier(const NetworkTimeSupplier& supplier) {
		SetOnce(m_networkTimeSupplier, supplier);
	}

	void ExtensionManager::addServiceRegistrar(std::unique_ptr<ServiceRegistrar>&& pServiceRegistrar) {
		m_serviceRegistrars.push_back(std::move(pServiceRegistrar));
	}

	const std::vector<std::string>& ExtensionManager::systemPluginNames() const {
		return m_systemPluginNames;
	}

	ExtensionManager::NetworkTimeSupplier ExtensionManager::networkTimeSupplier(const utils::TimeSpan& epochAdjustment) const {
		return m_networkTimeSupplier ? m_networkTimeSupplier : [epochAdjustment]() { return utils::NetworkTime(epochAdjustment).now(); };
	}

	void ExtensionManager::registerServices(ServiceLocator& locator, ServiceState& state) {
		// sort the registrars based on their annnounced phases
		std::sort(m_serviceRegistrars.begin(), m_serviceRegistrars.end(), [](const auto& pLhs, const auto& pRhs) {
			return utils::to_underlying_type(pLhs->info().Phase) < utils::to_underlying_type(pRhs->info().Phase);
		});

		// register counters first
		for (const auto& pRegistrar : m_serviceRegistrars)
			pRegistrar->registerServiceCounters(locator);

		// register services second
		for (const auto& pRegistrar : m_serviceRegistrars) {
			CATAPULT_LOG(debug) << "registering " << pRegistrar->info().Name << " services";
			pRegistrar->registerServices(locator, state);
		}

		// services are registered, no need to keep registrars around
		m_serviceRegistrars.clear();
	}
}}
