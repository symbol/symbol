#include "CacheTestUtils.h"
#include "plugins/coresystem/src/cache/AccountStateCacheStorage.h"
#include "plugins/services/blockdifficultycache/src/cache/BlockDifficultyCacheStorage.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache/SubCachePluginAdapter.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	namespace {
		const Key Sentinel_Cache_Public_Key = GenerateRandomData<Key_Size>();
	}

	// region CoreSystemCacheFactory

	cache::CatapultCache CoreSystemCacheFactory::Create(const model::BlockChainConfiguration& config) {
		std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(2);
		CreateSubCaches(config, subCaches);
		return cache::CatapultCache(std::move(subCaches));
	}

	void CoreSystemCacheFactory::CreateSubCaches(
			const model::BlockChainConfiguration& config,
			std::vector<std::unique_ptr<cache::SubCachePlugin>>& subCaches) {
		using namespace cache;
		subCaches[AccountStateCache::Id] = MakeSubCachePlugin<AccountStateCache, AccountStateCacheStorage>(
				config.Network.Identifier,
				config.ImportanceGrouping);
		subCaches[BlockDifficultyCache::Id] = MakeSubCachePlugin<BlockDifficultyCache, BlockDifficultyCacheStorage>(
				CalculateDifficultyHistorySize(config));
	}

	// endregion

	cache::CatapultCache CreateEmptyCatapultCache() {
		return CreateEmptyCatapultCache(model::BlockChainConfiguration::Uninitialized());
	}

	cache::CatapultCache CreateEmptyCatapultCache(const model::BlockChainConfiguration& config) {
		return CreateEmptyCatapultCache<CoreSystemCacheFactory>(config);
	}

	cache::CatapultCache CreateCatapultCacheWithMarkerAccount() {
		auto cache = CreateEmptyCatapultCache();
		auto delta = cache.createDelta();
		delta.sub<cache::AccountStateCache>().addAccount(Sentinel_Cache_Public_Key, Height(1));
		cache.commit(Height());
		return cache;
	}

	namespace {
		template<typename TCache>
		bool IsMarkedCacheT(TCache& cache) {
			const auto& accountStateCache = cache.template sub<cache::AccountStateCache>();
			return 1u == accountStateCache.size() && accountStateCache.contains(Sentinel_Cache_Public_Key);
		}
	}

	bool IsMarkedCache(const cache::ReadOnlyCatapultCache& cache) {
		return IsMarkedCacheT(cache);
	}

	bool IsMarkedCache(const cache::CatapultCacheDelta& cache) {
		return IsMarkedCacheT(cache);
	}
}}
