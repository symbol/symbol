#include "NamespaceCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		template<typename TCache>
		void AssertCacheSizesT(const TCache& cache, size_t size, size_t activeSize, size_t deepSize) {
			// Assert:
			EXPECT_EQ(size, cache.size());
			EXPECT_EQ(activeSize, cache.activeSize());
			EXPECT_EQ(deepSize, cache.deepSize());
		}
	}

	void AssertCacheSizes(const cache::NamespaceCacheView& cache, size_t size, size_t activeSize, size_t deepSize) {
		AssertCacheSizesT(cache, size, activeSize, deepSize);
	}

	void AssertCacheSizes(const cache::NamespaceCacheDelta& cache, size_t size, size_t activeSize, size_t deepSize) {
		AssertCacheSizesT(cache, size, activeSize, deepSize);
	}

	namespace {
		template<typename TCache>
		void AssertCacheContentsT(const TCache& cache, std::initializer_list<NamespaceId::ValueType> expectedIds) {
			// Assert:
			EXPECT_EQ(expectedIds.size(), cache.activeSize());
			for (auto id : expectedIds)
				EXPECT_TRUE(cache.contains(NamespaceId(id))) << "id " << id;
		}
	}

	void AssertCacheContents(const cache::NamespaceCache& cache, std::initializer_list<NamespaceId::ValueType> expectedIds) {
		auto view = cache.createView();
		AssertCacheContentsT(*view, expectedIds);
	}

	void AssertCacheContents(const cache::NamespaceCacheView& cache, std::initializer_list<NamespaceId::ValueType> expectedIds) {
		AssertCacheContentsT(cache, expectedIds);
	}

	void AssertCacheContents(const cache::NamespaceCacheDelta& cache, std::initializer_list<NamespaceId::ValueType> expectedIds) {
		AssertCacheContentsT(cache, expectedIds);
	}
}}
