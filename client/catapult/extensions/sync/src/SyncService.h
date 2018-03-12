#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace sync {

	/// Creates a registrar for a sync service.
	/// \note This service is responsible for allowing the node to sync with sync sources.
	DECLARE_SERVICE_REGISTRAR(Sync)();
}}
