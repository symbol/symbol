#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace partialtransaction {

	/// Creates a registrar for a partial transaction sync source service.
	/// \note This service is responsible for making the node a partial transaction sync partner.
	DECLARE_SERVICE_REGISTRAR(PtSyncSource)();
}}
