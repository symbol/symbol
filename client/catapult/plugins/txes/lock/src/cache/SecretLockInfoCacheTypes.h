#pragma once
#include "LockInfoCacheDelta.h"
#include "LockInfoCacheTypes.h"
#include "LockInfoCacheView.h"
#include "src/model/LockInfo.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"

namespace catapult {
	namespace cache {
		class SecretLockInfoCache;
		struct SecretLockInfoCacheTypes;
	}
}

namespace catapult { namespace cache {

	/// Describes a secret lock info cache.
	struct SecretLockInfoCacheDescriptor {
	public:
		// key value types
		using KeyType = Hash512;
		using ValueType = model::SecretLockInfo;

		// cache types
		using CacheType = SecretLockInfoCache;
		using CacheDeltaType = LockInfoCacheDelta<SecretLockInfoCacheDescriptor, SecretLockInfoCacheTypes>;
		using CacheViewType = LockInfoCacheView<SecretLockInfoCacheDescriptor, SecretLockInfoCacheTypes>;

	public:
		/// Gets the key corresponding to \a lockInfo.
		static const auto& GetKeyFromValue(const ValueType& lockInfo) {
			return lockInfo.Secret;
		}
	};

	/// Secret lock info cache types.
	struct SecretLockInfoCacheTypes : public LockInfoCacheTypes<SecretLockInfoCacheDescriptor> {
		using BasicCacheDeltaType = BasicLockInfoCacheDelta<SecretLockInfoCacheDescriptor, SecretLockInfoCacheTypes>;
		using BasicCacheViewType = BasicLockInfoCacheView<SecretLockInfoCacheDescriptor, SecretLockInfoCacheTypes>;

		using CacheReadOnlyType = ReadOnlyArtifactCache<BasicCacheViewType, BasicCacheDeltaType, Hash512, const model::SecretLockInfo&>;
	};
}}
