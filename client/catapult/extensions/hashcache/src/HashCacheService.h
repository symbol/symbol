#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace hashcache {

	/// Creates a registrar for a hash cache service.
	/// \note This service is responsible for registering the hash cache.
	DECLARE_SERVICE_REGISTRAR(HashCache)();
}}
