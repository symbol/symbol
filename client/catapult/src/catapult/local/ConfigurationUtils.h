#pragma once
#include "catapult/cache/MemoryUtCache.h"

namespace catapult { namespace config { struct NodeConfiguration; } }

namespace catapult { namespace local {

	/// Extracts unconfirmed transactions cache options from \a config.
	cache::MemoryUtCacheOptions GetUnconfirmedTransactionsCacheOptions(const config::NodeConfiguration& config);
}}
