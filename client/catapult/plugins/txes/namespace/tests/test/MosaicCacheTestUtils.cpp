#include "MosaicCacheTestUtils.h"
#include "MosaicTestUtils.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	void AddMosaic(cache::CatapultCacheDelta& cache, MosaicId id, Height height, BlockDuration duration, Amount supply) {
		auto& mosaicCacheDelta = cache.sub<cache::MosaicCache>();
		auto definition = state::MosaicDefinition(height, Key(), test::CreateMosaicPropertiesWithDuration(duration));
		auto entry = state::MosaicEntry(NamespaceId(11), id, definition);
		entry.increaseSupply(supply);
		mosaicCacheDelta.insert(entry);
	}

	void AddMosaic(
			cache::CatapultCacheDelta& cache,
			NamespaceId namespaceId,
			MosaicId id,
			Height height,
			BlockDuration duration,
			const Key& owner) {
		auto& mosaicCacheDelta = cache.sub<cache::MosaicCache>();
		auto definition = state::MosaicDefinition(height, owner, test::CreateMosaicPropertiesWithDuration(duration));
		mosaicCacheDelta.insert(state::MosaicEntry(namespaceId, id, definition));
	}

	void AddEternalMosaic(cache::CatapultCacheDelta& cache, NamespaceId namespaceId, MosaicId id, Height height) {
		AddEternalMosaic(cache, namespaceId, id, height, Key());
	}

	void AddEternalMosaic(cache::CatapultCacheDelta& cache, NamespaceId namespaceId, MosaicId id, Height height, const Key& owner) {
		auto& mosaicCacheDelta = cache.sub<cache::MosaicCache>();
		auto definition = state::MosaicDefinition(height, owner, model::MosaicProperties::FromValues({}));
		mosaicCacheDelta.insert(state::MosaicEntry(namespaceId, id, definition));
	}

	void AddMosaicOwner(cache::CatapultCacheDelta& cache, MosaicId id, const Key& owner, Amount amount) {
		auto& accountStateCacheDelta = cache.sub<cache::AccountStateCache>();
		accountStateCacheDelta.addAccount(owner, Height(1)).Balances.credit(id, amount);
	}

	namespace {
		template<typename TCache>
		void AssertCacheSizesT(const TCache& cache, size_t size, size_t deepSize) {
			// Assert:
			EXPECT_EQ(size, cache.size());
			EXPECT_EQ(deepSize, cache.deepSize());
		}
	}

	void AssertCacheSizes(const cache::MosaicCacheView& cache, size_t size, size_t deepSize) {
		AssertCacheSizesT(cache, size, deepSize);
	}

	void AssertCacheSizes(const cache::MosaicCacheDelta& cache, size_t size, size_t deepSize) {
		AssertCacheSizesT(cache, size, deepSize);
	}

	namespace {
		template<typename TCache>
		void AssertCacheContentsT(const TCache& cache, std::initializer_list<MosaicId::ValueType> expectedIds) {
			// Assert:
			EXPECT_EQ(expectedIds.size(), cache.size());
			for (auto id : expectedIds)
				EXPECT_TRUE(cache.contains(MosaicId(id))) << "id " << id;
		}
	}

	void AssertCacheContents(const cache::MosaicCache& cache, std::initializer_list<MosaicId::ValueType> expectedIds) {
		auto view = cache.createView();
		AssertCacheContentsT(*view, expectedIds);
	}

	void AssertCacheContents(const cache::MosaicCacheView& cache, std::initializer_list<MosaicId::ValueType> expectedIds) {
		AssertCacheContentsT(cache, expectedIds);
	}

	void AssertCacheContents(const cache::MosaicCacheDelta& cache, std::initializer_list<MosaicId::ValueType> expectedIds) {
		AssertCacheContentsT(cache, expectedIds);
	}
}}
