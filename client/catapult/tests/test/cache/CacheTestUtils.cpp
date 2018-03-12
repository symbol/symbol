#include "CacheTestUtils.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache/SubCachePluginAdapter.h"
#include "catapult/cache_core/AccountStateCacheStorage.h"
#include "catapult/cache_core/BlockDifficultyCacheStorage.h"
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

		auto accountStateCacheOptions = AccountStateCacheTypes::Options{
			config.Network.Identifier,
			config.ImportanceGrouping,
			config.MinHarvesterBalance
		};
		subCaches[AccountStateCache::Id] = MakeSubCachePlugin<AccountStateCache, AccountStateCacheStorage>(accountStateCacheOptions);

		auto difficultyHistorySize = CalculateDifficultyHistorySize(config);
		subCaches[BlockDifficultyCache::Id] = MakeSubCachePlugin<BlockDifficultyCache, BlockDifficultyCacheStorage>(difficultyHistorySize);
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
