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
#include "ServiceRegistrarPhase.h"
#include "catapult/plugins.h"
#include <memory>
#include <string>

namespace catapult {
	namespace extensions {
		class ServiceLocator;
		class ServiceState;
	}
}

namespace catapult { namespace extensions {

	/// Information about a service registrar.
	struct ServiceRegistrarInfo {
		/// Registrar friendly name.
		std::string Name;

		/// Phase during which the registrar should be invoked.
		ServiceRegistrarPhase Phase;
	};

	/// Registrar for registering a service.
	class PLUGIN_API_DEPENDENCY ServiceRegistrar {
	public:
		virtual ~ServiceRegistrar() = default;

	public:
		/// Gets information about the registrar.
		virtual ServiceRegistrarInfo info() const = 0;

		/// Registers service dependent counters with \a locator.
		virtual void registerServiceCounters(ServiceLocator& locator) = 0;

		/// Boots and registers dependent services with \a locator given \a state.
		virtual void registerServices(ServiceLocator& locator, ServiceState& state) = 0;
	};

/// Declares a service registrar entry point with \a NAME.
#define DECLARE_SERVICE_REGISTRAR(NAME) std::unique_ptr<extensions::ServiceRegistrar> Create##NAME##ServiceRegistrar
}}
