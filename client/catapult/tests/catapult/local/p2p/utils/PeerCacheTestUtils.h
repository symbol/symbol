#pragma once
#include "plugins/services/hashcache/src/cache/HashCacheStorage.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"

namespace catapult { namespace test {

	/// Cache factory for creating a catapult cache composed of all required peer caches.
	struct PeerCacheFactory {
		/// Creates an empty catapult cache.
		static cache::CatapultCache Create() {
			return Create(model::BlockChainConfiguration::Uninitialized());
		}

		/// Creates an empty catapult cache around \a config.
		static cache::CatapultCache Create(const model::BlockChainConfiguration& config) {
			auto cacheId = cache::HashCache::Id;
			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(cacheId + 1);
			CoreSystemCacheFactory::CreateSubCaches(config, subCaches);
			subCaches[cacheId] = MakeSubCachePlugin<cache::HashCache, cache::HashCacheStorage>(CalculateTransactionCacheDuration(config));
			return cache::CatapultCache(std::move(subCaches));
		}
	};
}}
