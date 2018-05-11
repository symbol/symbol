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
