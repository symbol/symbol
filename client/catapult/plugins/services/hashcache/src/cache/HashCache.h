#pragma once
#include "HashCacheDelta.h"
#include "HashCacheView.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	using HashBasicCache = BasicCache<HashCacheDescriptor, HashCacheTypes::BaseSetType, HashCacheTypes::Options>;

	/// Cache composed of timestamped hashes of (transaction) elements.
	/// \note The cache can be pruned according to the retention time.
	class BasicHashCache : public HashBasicCache {
	public:
		/// Creates a cache with the specified retention time (\a retentionTime).
		explicit BasicHashCache(const utils::TimeSpan& retentionTime)
				: HashBasicCache(HashCacheTypes::Options{ retentionTime })
		{}
	};

	/// Synchronized cache composed of timestamped hashes of (transaction) elements.
	class HashCache : public SynchronizedCache<BasicHashCache> {
	public:
		DEFINE_CACHE_CONSTANTS(Hash)

	public:
		/// Creates a cache with the specified retention time (\a retentionTime).
		explicit HashCache(const utils::TimeSpan& retentionTime)
				: SynchronizedCache<BasicHashCache>(BasicHashCache(retentionTime))
		{}
	};
}}
