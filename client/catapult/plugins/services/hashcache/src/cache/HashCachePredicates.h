#pragma once
#include "catapult/types.h"

namespace catapult { namespace cache { class CatapultCache; } }

namespace catapult { namespace cache {

	/// Returns \c true if \a cache contains \a hash with \a timestamp.
	bool HashCacheContains(const CatapultCache& cache, Timestamp timestamp, const Hash256& hash);
}}
