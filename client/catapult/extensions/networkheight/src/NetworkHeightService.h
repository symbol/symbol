#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace networkheight {

	/// Creates a registrar for a network height service.
	/// \note This service is responsible for calculating the network height.
	DECLARE_SERVICE_REGISTRAR(NetworkHeight)();
}}
