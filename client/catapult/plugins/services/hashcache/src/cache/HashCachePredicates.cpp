#include "HashCachePredicates.h"
#include "HashCache.h"
#include "catapult/cache/CatapultCache.h"

namespace catapult { namespace cache {

	bool HashCacheContains(const CatapultCache& cache, Timestamp timestamp, const Hash256& hash) {
		const auto& hashCache = cache.sub<HashCache>();
		return hashCache.createView()->contains(state::TimestampedHash(timestamp, hash));
	}
}}
