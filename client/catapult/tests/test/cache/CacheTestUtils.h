#pragma once
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/SubCachePluginAdapter.h"
#include <memory>
#include <vector>

namespace catapult { namespace cache { class ReadOnlyCatapultCache; } }

namespace catapult { namespace test {

	/// Cache factory for creating a catapult cache composed of all core subcaches.
	struct CoreSystemCacheFactory {
		/// Creates an empty catapult cache around \a config.
		static cache::CatapultCache Create(const model::BlockChainConfiguration& config);

		/// Adds all core subcaches initialized with \a config to \a subCaches.
		static void CreateSubCaches(
				const model::BlockChainConfiguration& config,
				std::vector<std::unique_ptr<cache::SubCachePlugin>>& subCaches);
	};

	/// Creates a subcache plugin given \a args.
	template<typename TCache, typename TStorageTraits, typename... TArgs>
	std::unique_ptr<cache::SubCachePlugin> MakeSubCachePlugin(TArgs&&... args) {
		auto pCache = std::make_unique<TCache>(std::forward<TArgs>(args)...);
		return std::make_unique<cache::SubCachePluginAdapter<TCache, TStorageTraits>>(std::move(pCache));
	}

	/// Creates an empty catapult cache.
	cache::CatapultCache CreateEmptyCatapultCache();

	/// Creates an empty catapult cache around \a config.
	cache::CatapultCache CreateEmptyCatapultCache(const model::BlockChainConfiguration& config);

	/// Creates an empty catapult cache around \a config.
	template<typename TCacheFactory>
	cache::CatapultCache CreateEmptyCatapultCache(const model::BlockChainConfiguration& config) {
		return TCacheFactory::Create(config);
	}

	/// Creates a catapult cache with a marker account.
	cache::CatapultCache CreateCatapultCacheWithMarkerAccount();

	/// Returns \c true if \a cache contains the marker account.
	bool IsMarkedCache(const cache::ReadOnlyCatapultCache& cache);

	/// Returns \c true if \a cache contains the marker account.
	bool IsMarkedCache(const cache::CatapultCacheDelta& cache);

	/// Extract all values from \a cache.
	template<typename TCache, typename TValue>
	std::vector<TValue> ExtractValuesFromCache(const TCache& cache) {
		std::vector<TValue> values;
		auto view = cache.createView();
		for (const auto& pair : *view)
			values.push_back(pair.second);

		return values;
	}
}}
