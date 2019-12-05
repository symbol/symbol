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

#include "harvesting/src/HarvestingCacheUtils.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS HarvestingCacheUtilsTests

	namespace {
		template<typename TAction>
		void RunPreserveAllAccountsTest(TAction action) {
			// Arrange:
			cache::AccountStateCache accountStateCache(cache::CacheConfiguration(), test::CreateDefaultAccountStateCacheOptions());
			auto accountStateCacheDelta = accountStateCache.createDelta();

			auto addresses = test::GenerateRandomDataVector<Address>(3);
			for (const auto& address : addresses) {
				accountStateCacheDelta->addAccount(address, Height(6));
				accountStateCacheDelta->queueRemove(address, Height(6));
			}

			auto publicKeys = test::GenerateRandomDataVector<Key>(3);
			for (const auto& publicKey : publicKeys) {
				accountStateCacheDelta->addAccount(publicKey, Height(6));
				accountStateCacheDelta->queueRemove(publicKey, Height(6));
			}

			// Sanity:
			EXPECT_EQ(6u, accountStateCacheDelta->size());

			// Act:
			action(*accountStateCacheDelta, addresses, publicKeys);
		}
	}

	TEST(TEST_CLASS, PreserveAllAccounts_CanPreserveZeroAccounts) {
		// Arrange:
		RunPreserveAllAccountsTest([](auto& accountStateCacheDelta, const auto&, const auto&) {
			HarvestingAffectedAccounts accounts;

			// Act:
			PreserveAllAccounts(accountStateCacheDelta, accounts, Height(6));
			accountStateCacheDelta.commitRemovals();

			// Assert:
			EXPECT_EQ(0u, accountStateCacheDelta.size());
		});
	}

	TEST(TEST_CLASS, PreserveAllAccounts_CanPreserveAddresses) {
		// Arrange:
		RunPreserveAllAccountsTest([](auto& accountStateCacheDelta, const auto& addresses, const auto&) {
			HarvestingAffectedAccounts accounts;
			accounts.Addresses.emplace(addresses[0], 1);
			accounts.Addresses.emplace(addresses[2], 3);

			// Act:
			PreserveAllAccounts(accountStateCacheDelta, accounts, Height(6));
			accountStateCacheDelta.commitRemovals();

			// Assert:
			EXPECT_EQ(2u, accountStateCacheDelta.size());
			EXPECT_TRUE(accountStateCacheDelta.contains(addresses[0]));
			EXPECT_TRUE(accountStateCacheDelta.contains(addresses[2]));
		});
	}

	TEST(TEST_CLASS, PreserveAllAccounts_CanPreservePublicKeys) {
		// Arrange:
		RunPreserveAllAccountsTest([](auto& accountStateCacheDelta, const auto&, const auto& publicKeys) {
			HarvestingAffectedAccounts accounts;
			accounts.PublicKeys.emplace(publicKeys[0], 1);
			accounts.PublicKeys.emplace(publicKeys[2], 3);

			// Act:
			PreserveAllAccounts(accountStateCacheDelta, accounts, Height(6));
			accountStateCacheDelta.commitRemovals();

			// Assert:
			EXPECT_EQ(2u, accountStateCacheDelta.size());
			EXPECT_TRUE(accountStateCacheDelta.contains(publicKeys[0]));
			EXPECT_TRUE(accountStateCacheDelta.contains(publicKeys[2]));
		});
	}

	TEST(TEST_CLASS, PreserveAllAccounts_CanPreserveAddressesAndPublicKeys) {
		// Arrange:
		RunPreserveAllAccountsTest([](auto& accountStateCacheDelta, const auto& addresses, const auto& publicKeys) {
			HarvestingAffectedAccounts accounts;
			accounts.Addresses.emplace(addresses[1], 2);
			accounts.PublicKeys.emplace(publicKeys[0], 1);
			accounts.PublicKeys.emplace(publicKeys[2], 3);

			// Act:
			PreserveAllAccounts(accountStateCacheDelta, accounts, Height(6));
			accountStateCacheDelta.commitRemovals();

			// Assert:
			EXPECT_EQ(3u, accountStateCacheDelta.size());
			EXPECT_TRUE(accountStateCacheDelta.contains(addresses[1]));
			EXPECT_TRUE(accountStateCacheDelta.contains(publicKeys[0]));
			EXPECT_TRUE(accountStateCacheDelta.contains(publicKeys[2]));
		});
	}
}}
