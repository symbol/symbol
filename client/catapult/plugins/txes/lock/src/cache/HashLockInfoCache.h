#pragma once
#include "HashLockInfoCacheTypes.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	/// Cache composed of hash lock info information.
	using BasicHashLockInfoCache = BasicCache<HashLockInfoCacheDescriptor, HashLockInfoCacheTypes::BaseSetType>;

	/// Synchronized cache composed of hash lock info information.
	class HashLockInfoCache : public SynchronizedCache<BasicHashLockInfoCache> {
	public:
		DEFINE_CACHE_CONSTANTS(HashLockInfo)

	public:
		/// Creates a cache.
		HashLockInfoCache() : SynchronizedCache<BasicHashLockInfoCache>(BasicHashLockInfoCache())
		{}
	};
}}
