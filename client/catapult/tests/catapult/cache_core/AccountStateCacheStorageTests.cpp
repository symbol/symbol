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

#include "catapult/cache_core/AccountStateCacheStorage.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountStateCacheStorageTests

	namespace {
		constexpr auto Default_Cache_Options = AccountStateCacheTypes::Options{
			model::NetworkIdentifier::Mijin_Test,
			543,
			Amount(std::numeric_limits<Amount::ValueType>::max()),
			MosaicId(1111),
			MosaicId(2222)
		};
	}

	TEST(TEST_CLASS, CanLoadValueIntoCache) {
		// Arrange: create a random value to insert
		state::AccountState originalAccountState(test::GenerateRandomAddress(), Height(111));
		test::RandomFillAccountData(0, originalAccountState, 123);

		// Act:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		{
			auto delta = cache.createDelta();
			AccountStateCacheStorage::LoadInto(originalAccountState, *delta);
			cache.commit();
		}

		// Assert: the cache contains the value
		auto view = cache.createView();
		EXPECT_EQ(1u, view->size());
		ASSERT_TRUE(view->contains(originalAccountState.Address));
		const auto& loadedAccountState = view->find(originalAccountState.Address).get();

		// - the loaded cache value is correct
		EXPECT_EQ(123u, loadedAccountState.Balances.size());

		// - cache automatically optimizes added account state, so update to match expected
		originalAccountState.Balances.optimize(Default_Cache_Options.CurrencyMosaicId);
		test::AssertEqual(originalAccountState, loadedAccountState);
	}
}}
