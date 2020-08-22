/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "CacheTestUtils.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache/SubCachePluginAdapter.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/AccountStateCacheStorage.h"
#include "catapult/cache_core/BlockStatisticCacheStorage.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	namespace {
		Key GetSentinelCachePublicKey() {
			return { { 0xFF, 0xFF, 0xFF, 0xFF } };
		}

		cache::AccountStateCacheTypes::Options CreateAccountStateCacheOptions(const model::BlockChainConfiguration& config) {
			return {
				config.Network.Identifier,
				config.ImportanceGrouping,
				config.VotingSetGrouping,
				config.MinHarvesterBalance,
				config.MaxHarvesterBalance,
				config.MinVoterBalance,
				config.CurrencyMosaicId,
				config.HarvestingMosaicId
			};
		}
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
		CreateSubCaches(config, cache::CacheConfiguration(), subCaches);
	}

	void CoreSystemCacheFactory::CreateSubCaches(
			const model::BlockChainConfiguration& config,
			const cache::CacheConfiguration& cacheConfig,
			std::vector<std::unique_ptr<cache::SubCachePlugin>>& subCaches) {
		using namespace cache;

		subCaches[AccountStateCache::Id] = MakeSubCachePluginWithCacheConfiguration<AccountStateCache, AccountStateCacheStorage>(
				cacheConfig,
				CreateAccountStateCacheOptions(config));

		subCaches[BlockStatisticCache::Id] = MakeConfigurationFreeSubCachePlugin<BlockStatisticCache, BlockStatisticCacheStorage>(
				CalculateDifficultyHistorySize(config));
	}

	// endregion

	// region CreateEmptyCatapultCache

	cache::CatapultCache CreateEmptyCatapultCache() {
		auto config = model::BlockChainConfiguration::Uninitialized();
		config.VotingSetGrouping = 1;
		return CreateEmptyCatapultCache(config);
	}

	cache::CatapultCache CreateEmptyCatapultCache(const model::BlockChainConfiguration& config) {
		return CreateEmptyCatapultCache<CoreSystemCacheFactory>(config);
	}

	cache::CatapultCache CreateEmptyCatapultCache(
			const model::BlockChainConfiguration& config,
			const cache::CacheConfiguration& cacheConfig) {
		std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(2);
		CoreSystemCacheFactory::CreateSubCaches(config, cacheConfig, subCaches);
		return cache::CatapultCache(std::move(subCaches));
	}

	// endregion

	// region cache marker utils

	cache::CatapultCache CreateCatapultCacheWithMarkerAccount() {
		return CreateCatapultCacheWithMarkerAccount(Height(0));
	}

	cache::CatapultCache CreateCatapultCacheWithMarkerAccount(Height height) {
		auto cache = CreateEmptyCatapultCache();
		AddMarkerAccount(cache);

		auto delta = cache.createDelta();
		cache.commit(height);
		return cache;
	}

	void AddMarkerAccount(cache::CatapultCache& cache) {
		auto delta = cache.createDelta();
		delta.sub<cache::AccountStateCache>().addAccount(GetSentinelCachePublicKey(), Height(1));
		cache.commit(Height(1));
	}

	namespace {
		template<typename TCache>
		bool IsMarkedCacheT(TCache& cache, IsMarkedCacheMode mode) {
			const auto& accountStateCache = cache.template sub<cache::AccountStateCache>();
			if (IsMarkedCacheMode::Exclusive == mode && 1 != accountStateCache.size())
				return false;

			return accountStateCache.contains(GetSentinelCachePublicKey());
		}
	}

	bool IsMarkedCache(const cache::ReadOnlyCatapultCache& cache, IsMarkedCacheMode mode) {
		return IsMarkedCacheT(cache, mode);
	}

	bool IsMarkedCache(const cache::CatapultCacheDelta& cache, IsMarkedCacheMode mode) {
		return IsMarkedCacheT(cache, mode);
	}

	// endregion
}}
