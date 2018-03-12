#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace syncsource {

	/// Creates a registrar for a sync source service.
	/// \note This service is responsible for making the node a sync partner.
	DECLARE_SERVICE_REGISTRAR(SyncSource)();
}}
