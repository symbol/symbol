#pragma once
#include "SecretLockInfoCacheTypes.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	/// Cache composed of secret lock info information.
	using BasicSecretLockInfoCache = BasicCache<SecretLockInfoCacheDescriptor, SecretLockInfoCacheTypes::BaseSetType>;

	/// Synchronized cache composed of secret lock info information.
	class SecretLockInfoCache : public SynchronizedCache<BasicSecretLockInfoCache> {
	public:
		DEFINE_CACHE_CONSTANTS(SecretLockInfo)

	public:
		/// Creates a cache.
		SecretLockInfoCache() : SynchronizedCache<BasicSecretLockInfoCache>(BasicSecretLockInfoCache())
		{}
	};
}}
