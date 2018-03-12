#pragma once
#include "LockInfoCacheDelta.h"
#include "LockInfoCacheTypes.h"
#include "LockInfoCacheView.h"
#include "src/model/LockInfo.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"

namespace catapult {
	namespace cache {
		class HashLockInfoCache;
		struct HashLockInfoCacheTypes;
	}
}

namespace catapult { namespace cache {

	/// Describes a hash lock info cache.
	struct HashLockInfoCacheDescriptor {
	public:
		// key value types
		using KeyType = Hash256;
		using ValueType = model::HashLockInfo;

		// cache types
		using CacheType = HashLockInfoCache;
		using CacheDeltaType = LockInfoCacheDelta<HashLockInfoCacheDescriptor, HashLockInfoCacheTypes>;
		using CacheViewType = LockInfoCacheView<HashLockInfoCacheDescriptor, HashLockInfoCacheTypes>;

	public:
		/// Gets the key corresponding to \a lockInfo.
		static const auto& GetKeyFromValue(const ValueType& lockInfo) {
			return lockInfo.Hash;
		}
	};

	/// Hash lock info cache types.
	struct HashLockInfoCacheTypes : public LockInfoCacheTypes<HashLockInfoCacheDescriptor> {
		using BasicCacheDeltaType = BasicLockInfoCacheDelta<HashLockInfoCacheDescriptor, HashLockInfoCacheTypes>;
		using BasicCacheViewType = BasicLockInfoCacheView<HashLockInfoCacheDescriptor, HashLockInfoCacheTypes>;

		using CacheReadOnlyType = ReadOnlyArtifactCache<BasicCacheViewType, BasicCacheDeltaType, Hash256, const model::HashLockInfo&>;
	};
}}
