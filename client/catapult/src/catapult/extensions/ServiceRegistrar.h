#pragma once
#include "ServiceRegistrarPhase.h"
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
		/// The registrar friendly name.
		std::string Name;

		/// The phase during which the registrar should be invoked.
		ServiceRegistrarPhase Phase;
	};

	/// A registrar for registering a service.
	class ServiceRegistrar {
	public:
		virtual ~ServiceRegistrar() {}

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
