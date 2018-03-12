#pragma once
#include "HarvestingConfiguration.h"
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace harvesting {

	/// Creates a registrar for a harvesting service around \a config.
	/// \note This service is responsible for enabling node harvesting.
	DECLARE_SERVICE_REGISTRAR(Harvesting)(const HarvestingConfiguration& config);
}}
