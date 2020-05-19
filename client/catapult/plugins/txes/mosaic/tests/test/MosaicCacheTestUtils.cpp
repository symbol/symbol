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
		AddMosaic(cache, id, height, duration, supply, Address());
	}

	void AddMosaic(cache::CatapultCacheDelta& cache, MosaicId id, Height height, BlockDuration duration, const Address& owner) {
		AddMosaic(cache, id, height, duration, Amount(), owner);
	}

	void AddMosaic(
			cache::CatapultCacheDelta& cache,
			MosaicId id,
			Height height,
			BlockDuration duration,
			Amount supply,
			const Address& owner) {
		auto& mosaicCacheDelta = cache.sub<cache::MosaicCache>();
		auto definition = state::MosaicDefinition(height, owner, 1, test::CreateMosaicPropertiesWithDuration(duration));
		auto mosaicEntry = state::MosaicEntry(id, definition);
		mosaicEntry.increaseSupply(supply);
		mosaicCacheDelta.insert(mosaicEntry);
	}

	void AddEternalMosaic(cache::CatapultCacheDelta& cache, MosaicId id, Height height) {
		AddEternalMosaic(cache, id, height, Address());
	}

	void AddEternalMosaic(cache::CatapultCacheDelta& cache, MosaicId id, Height height, const Address& owner) {
		auto& mosaicCacheDelta = cache.sub<cache::MosaicCache>();
		auto definition = state::MosaicDefinition(height, owner, 1, model::MosaicProperties());
		mosaicCacheDelta.insert(state::MosaicEntry(id, definition));
	}

	void AddMosaicOwner(cache::CatapultCacheDelta& cache, MosaicId id, const Address& owner, Amount amount) {
		auto& accountStateCacheDelta = cache.sub<cache::AccountStateCache>();
		accountStateCacheDelta.addAccount(owner, Height(1));
		accountStateCacheDelta.find(owner).get().Balances.credit(id, amount);
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
