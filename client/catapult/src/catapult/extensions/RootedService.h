#pragma once
#include "ServiceRegistrar.h"

namespace catapult { namespace extensions {

	/// Creates a rooted service registrar that runs during the specified registration \a phase
	/// and simply registers \a pService as a rooted service with name \a serviceName.
	DECLARE_SERVICE_REGISTRAR(Rooted)(const std::shared_ptr<void>& pService, const std::string& serviceName, ServiceRegistrarPhase phase);
}}
