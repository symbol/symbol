#pragma once
#include "src/cache/NamespaceCacheStorage.h"
#include "tests/test/cache/CacheTestUtils.h"

namespace catapult { namespace test {

	/// Cache factory for creating a catapult cache composed of only the namespace cache.
	struct NamespaceCacheFactory {
		/// Creates an empty catapult cache.
		static cache::CatapultCache Create() {
			auto cacheId = cache::NamespaceCache::Id;
			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(cacheId + 1);
			subCaches[cacheId] = MakeSubCachePlugin<cache::NamespaceCache, cache::NamespaceCacheStorage>();
			return cache::CatapultCache(std::move(subCaches));
		}

		/// Creates an empty catapult cache around \a config.
		static cache::CatapultCache Create(const model::BlockChainConfiguration&) {
			return Create();
		}
	};

	/// Asserts that \a cache has the expected sizes (\a size, \a activeSize and \a deepSize).
	void AssertCacheSizes(const cache::NamespaceCacheView& cache, size_t size, size_t activeSize, size_t deepSize);

	/// Asserts that \a cache has the expected sizes (\a size, \a activeSize and \a deepSize).
	void AssertCacheSizes(const cache::NamespaceCacheDelta& cache, size_t size, size_t activeSize, size_t deepSize);

	/// Asserts that \a cache exactly contains the namespace ids in \a expectedIds.
	void AssertCacheContents(const cache::NamespaceCache& cache, std::initializer_list<NamespaceId::ValueType> expectedIds);

	/// Asserts that \a cache exactly contains the namespace ids in \a expectedIds.
	void AssertCacheContents(const cache::NamespaceCacheView& cache, std::initializer_list<NamespaceId::ValueType> expectedIds);

	/// Asserts that \a cache exactly contains the namespace ids in \a expectedIds.
	void AssertCacheContents(const cache::NamespaceCacheDelta& cache, std::initializer_list<NamespaceId::ValueType> expectedIds);
}}
