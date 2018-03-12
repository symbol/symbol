#pragma once
#include "src/cache/MultisigCache.h"
#include "src/cache/MultisigCacheStorage.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"

namespace catapult { namespace test {

	/// Cache factory for creating a catapult cache composed of multisig cache and core caches.
	struct MultisigCacheFactory {
	private:
		static auto CreateSubCachesWithMultisigCache() {
			auto cacheId = cache::MultisigCache::Id;
			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(cacheId + 1);
			subCaches[cacheId] = MakeSubCachePlugin<cache::MultisigCache, cache::MultisigCacheStorage>();
			return subCaches;
		}

	public:
		/// Creates an empty catapult cache around default configuration.
		static cache::CatapultCache Create() {
			return Create(model::BlockChainConfiguration::Uninitialized());
		}

		/// Creates an empty catapult cache around \a config.
		static cache::CatapultCache Create(const model::BlockChainConfiguration& config) {
			auto subCaches = CreateSubCachesWithMultisigCache();
			CoreSystemCacheFactory::CreateSubCaches(config, subCaches);
			return cache::CatapultCache(std::move(subCaches));
		}
	};
}}
