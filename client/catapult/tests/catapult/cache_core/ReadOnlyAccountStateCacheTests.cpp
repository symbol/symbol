/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/cache_core/ReadOnlyAccountStateCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS ReadOnlyAccountStateCacheTests

	namespace {
		constexpr auto Currency_Mosaic_Id = MosaicId(1234);
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);
		constexpr auto Default_Cache_Options = test::CreateDefaultAccountStateCacheOptions(Currency_Mosaic_Id, Harvesting_Mosaic_Id);
	}

	// region cache properties

	TEST(TEST_CLASS, NetworkIdentifierIsExposed) {
		// Arrange:
		auto networkIdentifier = static_cast<model::NetworkIdentifier>(19);
		auto options = Default_Cache_Options;
		options.NetworkIdentifier = networkIdentifier;
		AccountStateCache originalCache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(networkIdentifier, ReadOnlyAccountStateCache(*originalCache.createView()).networkIdentifier());
		EXPECT_EQ(networkIdentifier, ReadOnlyAccountStateCache(*originalCache.createDelta()).networkIdentifier());
	}

	TEST(TEST_CLASS, ImportanceGroupingIsExposed) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.ImportanceGrouping = 123;
		AccountStateCache originalCache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(123u, ReadOnlyAccountStateCache(*originalCache.createView()).importanceGrouping());
		EXPECT_EQ(123u, ReadOnlyAccountStateCache(*originalCache.createDelta()).importanceGrouping());
	}

	TEST(TEST_CLASS, MinHarvesterBalanceIsExposed) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.MinHarvesterBalance = Amount(336644);
		AccountStateCache originalCache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(Amount(336644), ReadOnlyAccountStateCache(*originalCache.createView()).minHarvesterBalance());
		EXPECT_EQ(Amount(336644), ReadOnlyAccountStateCache(*originalCache.createDelta()).minHarvesterBalance());
	}

	TEST(TEST_CLASS, MaxHarvesterBalanceIsExposed) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.MaxHarvesterBalance = Amount(446633);
		AccountStateCache originalCache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(Amount(446633), ReadOnlyAccountStateCache(*originalCache.createView()).maxHarvesterBalance());
		EXPECT_EQ(Amount(446633), ReadOnlyAccountStateCache(*originalCache.createDelta()).maxHarvesterBalance());
	}

	TEST(TEST_CLASS, HarvestingMosaicIdIsExposed) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.HarvestingMosaicId = MosaicId(11229988);
		AccountStateCache originalCache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(MosaicId(11229988), ReadOnlyAccountStateCache(*originalCache.createView()).harvestingMosaicId());
		EXPECT_EQ(MosaicId(11229988), ReadOnlyAccountStateCache(*originalCache.createDelta()).harvestingMosaicId());
	}

	// endregion

	// region size / contains / find

	namespace {
		struct AccountStateCacheByAddressTraits {
			static Address CreateKey(uint8_t tag) {
				return { { static_cast<uint8_t>(tag * tag) } };
			}

			static Address GetKey(const state::AccountState& accountState) {
				return accountState.Address;
			}
		};

		struct AccountStateCacheByKeyTraits {
			static Key CreateKey(uint8_t tag) {
				return { { static_cast<uint8_t>(tag * tag) } };
			}

			static Key GetKey(const state::AccountState& accountState) {
				return accountState.PublicKey;
			}
		};
	}

#define ACCOUNT_KEY_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ByAddress) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountStateCacheByAddressTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ByKey) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountStateCacheByKeyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ACCOUNT_KEY_BASED_TEST(ReadOnlyViewOnlyContainsCommittedElements) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		{
			auto cacheDelta = cache.createDelta();
			cacheDelta->addAccount(TTraits::CreateKey(1), Height(123)); // committed
			cache.commit();
			cacheDelta->addAccount(TTraits::CreateKey(2), Height(123)); // uncommitted
		}

		// Act:
		auto cacheView = cache.createView();
		ReadOnlyAccountStateCache readOnlyCache(*cacheView);

		// Assert:
		EXPECT_EQ(1u, readOnlyCache.size());
		EXPECT_TRUE(readOnlyCache.contains(TTraits::CreateKey(1)));
		EXPECT_FALSE(readOnlyCache.contains(TTraits::CreateKey(2)));
		EXPECT_FALSE(readOnlyCache.contains(TTraits::CreateKey(3)));
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyDeltaContainsBothCommittedAndUncommittedElements) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto cacheDelta = cache.createDelta();
		cacheDelta->addAccount(TTraits::CreateKey(1), Height(123)); // committed
		cache.commit();
		cacheDelta->addAccount(TTraits::CreateKey(2), Height(123)); // uncommitted

		// Act:
		ReadOnlyAccountStateCache readOnlyCache(*cacheDelta);

		// Assert:
		EXPECT_EQ(2u, readOnlyCache.size());
		EXPECT_TRUE(readOnlyCache.contains(TTraits::CreateKey(1)));
		EXPECT_TRUE(readOnlyCache.contains(TTraits::CreateKey(2)));
		EXPECT_FALSE(readOnlyCache.contains(TTraits::CreateKey(3)));
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyViewOnlyCanAccessCommittedElementsViaGet) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		{
			auto cacheDelta = cache.createDelta();
			cacheDelta->addAccount(TTraits::CreateKey(1), Height(123)); // committed;
			cache.commit();
			cacheDelta->addAccount(TTraits::CreateKey(2), Height(123)); // uncommitted
		}

		// Act:
		auto cacheView = cache.createView();
		ReadOnlyAccountStateCache readOnlyCache(*cacheView);

		// Assert:
		EXPECT_EQ(1u, readOnlyCache.size());
		EXPECT_EQ(TTraits::CreateKey(1), TTraits::GetKey(readOnlyCache.find(TTraits::CreateKey(1)).get()));
		EXPECT_THROW(readOnlyCache.find(TTraits::CreateKey(2)).get(), catapult_invalid_argument);
		EXPECT_THROW(readOnlyCache.find(TTraits::CreateKey(3)).get(), catapult_invalid_argument);
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyDeltaCanAccessBothCommittedAndUncommittedElementsViaGet) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto cacheDelta = cache.createDelta();
		cacheDelta->addAccount(TTraits::CreateKey(1), Height(123)); // committed
		cache.commit();
		cacheDelta->addAccount(TTraits::CreateKey(2), Height(123)); // uncommitted

		// Act:
		ReadOnlyAccountStateCache readOnlyCache(*cacheDelta);

		// Assert:
		EXPECT_EQ(2u, readOnlyCache.size());
		EXPECT_EQ(TTraits::CreateKey(1), TTraits::GetKey(readOnlyCache.find(TTraits::CreateKey(1)).get()));
		EXPECT_EQ(TTraits::CreateKey(2), TTraits::GetKey(readOnlyCache.find(TTraits::CreateKey(2)).get()));
		EXPECT_THROW(readOnlyCache.find(TTraits::CreateKey(3)).get(), catapult_invalid_argument);
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyViewOnlyCanAccessCommittedElementsViaTryGet) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		{
			auto cacheDelta = cache.createDelta();
			cacheDelta->addAccount(TTraits::CreateKey(1), Height(123)); // committed;
			cache.commit();
			cacheDelta->addAccount(TTraits::CreateKey(2), Height(123)); // uncommitted
		}

		// Act:
		auto cacheView = cache.createView();
		ReadOnlyAccountStateCache readOnlyCache(*cacheView);

		// Assert:
		EXPECT_EQ(1u, readOnlyCache.size());
		EXPECT_EQ(TTraits::CreateKey(1), TTraits::GetKey(*readOnlyCache.find(TTraits::CreateKey(1)).tryGet()));
		EXPECT_FALSE(!!readOnlyCache.find(TTraits::CreateKey(2)).tryGet());
		EXPECT_FALSE(!!readOnlyCache.find(TTraits::CreateKey(3)).tryGet());
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyDeltaCanAccessBothCommittedAndUncommittedElementsViaTryGet) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto cacheDelta = cache.createDelta();
		cacheDelta->addAccount(TTraits::CreateKey(1), Height(123)); // committed
		cache.commit();
		cacheDelta->addAccount(TTraits::CreateKey(2), Height(123)); // uncommitted

		// Act:
		ReadOnlyAccountStateCache readOnlyCache(*cacheDelta);

		// Assert:
		EXPECT_EQ(2u, readOnlyCache.size());
		EXPECT_EQ(TTraits::CreateKey(1), TTraits::GetKey(*readOnlyCache.find(TTraits::CreateKey(1)).tryGet()));
		EXPECT_EQ(TTraits::CreateKey(2), TTraits::GetKey(*readOnlyCache.find(TTraits::CreateKey(2)).tryGet()));
		EXPECT_FALSE(!!readOnlyCache.find(TTraits::CreateKey(3)).tryGet());
	}

	// endregion

	// region highValueAccountStatistics - basic

	namespace {
		std::vector<Address> AddAccountsWithBalances(AccountStateCacheDelta& delta, const std::vector<Amount>& balances) {
			return test::AddAccountsWithBalances(delta, Harvesting_Mosaic_Id, balances);
		}

		template<typename TDeltaAction, typename TViewAction>
		void RunHighValueAddressesTest(const std::vector<Amount>& balances, TDeltaAction deltaAction, TViewAction viewAction) {
			// Arrange: set min balances
			auto options = Default_Cache_Options;
			options.MinHarvesterBalance = Amount(1'000'000);
			options.MinVoterBalance = Amount(1'100'000);
			AccountStateCache cache(CacheConfiguration(), options);

			// - prepare delta with requested accounts
			std::vector<Address> addresses;
			{
				{
					auto delta = cache.createDelta();
					addresses = AddAccountsWithBalances(*delta, balances);
					delta->updateHighValueAccounts(Height(1));
					cache.commit();
				}

				// Act + Assert: run the test (need a new delta because commit is destructive)
				auto delta = cache.createDelta();
				deltaAction(addresses, delta);
			}

			// Assert: check the view
			viewAction(addresses, cache.createView());
		}
	}

	TEST(TEST_CLASS, HighValueAccountStatistics_ReturnsEmptySetWhenNoAccountsMeetCriteria) {
		// Arrange:
		auto deltaAction = [](const auto&, auto& delta) {
			// Act:
			delta->updateHighValueAccounts(Height(1));
			auto statistics = ReadOnlyAccountStateCache(*delta).highValueAccountStatistics();

			// Assert:
			EXPECT_EQ(0u, statistics.VotingEligibleAccountsCount);
			EXPECT_EQ(0u, statistics.HarvestingEligibleAccountsCount);
			EXPECT_EQ(Amount(), statistics.TotalVotingBalance);
		};
		auto viewAction = [](const auto&, const auto& view) {
			// Act:
			auto statistics = ReadOnlyAccountStateCache(*view).highValueAccountStatistics();

			// Assert:
			EXPECT_EQ(0u, statistics.VotingEligibleAccountsCount);
			EXPECT_EQ(0u, statistics.HarvestingEligibleAccountsCount);
			EXPECT_EQ(Amount(), statistics.TotalVotingBalance);
		};

		// - add 0/3 with sufficient balance
		auto balances = std::vector<Amount>{ Amount(999'999), Amount(1'000), Amount(1) };
		RunHighValueAddressesTest(balances, deltaAction, viewAction);
	}

	TEST(TEST_CLASS, HighValueAccountStatistics_ReturnsAllAccountsMeetingCriteria) {
		// Arrange:
		auto deltaAction = [](const auto& addresses, auto& delta) {
			// - add 3/4 accounts with sufficient balance (uncommitted) [7 match]
			auto uncommittedAddresses = AddAccountsWithBalances(*delta, {
				Amount(1'100'000), Amount(900'000), Amount(1'000'000), Amount(1'500'000)
			});

			// - modify two [7 match]
			delta->find(addresses[1]).get().Balances.credit(Harvesting_Mosaic_Id, Amount(500'000));
			delta->find(addresses[4]).get().Balances.debit(Harvesting_Mosaic_Id, Amount(200'001));

			// - delete two [5 match]
			delta->queueRemove(addresses[2], Height(1));
			delta->queueRemove(uncommittedAddresses[0], Height(1));
			delta->commitRemovals();

			// Act:
			delta->updateHighValueAccounts(Height(1));
			auto statistics = ReadOnlyAccountStateCache(*delta).highValueAccountStatistics();

			// Assert:
			EXPECT_EQ(4u, statistics.VotingEligibleAccountsCount);
			EXPECT_EQ(5u, statistics.HarvestingEligibleAccountsCount);
			EXPECT_EQ(Amount(8'000'000), statistics.TotalVotingBalance);
		};
		auto viewAction = [](const auto&, const auto& view) {
			// Act:
			auto statistics = ReadOnlyAccountStateCache(*view).highValueAccountStatistics();

			// Assert:
			EXPECT_EQ(3u, statistics.VotingEligibleAccountsCount);
			EXPECT_EQ(4u, statistics.HarvestingEligibleAccountsCount);
			EXPECT_EQ(Amount(6'300'000), statistics.TotalVotingBalance);
		};

		// - add 4/6 accounts with sufficient balance [4 match]
		auto balances = std::vector<Amount>{
			Amount(1'100'000), Amount(900'000), Amount(1'000'000), Amount(800'000), Amount(1'200'000), Amount(4'000'000)
		};
		RunHighValueAddressesTest(balances, deltaAction, viewAction);
	}

	// endregion

	// region highValueAccountStatistics - voting eligibility

	TEST(TEST_CLASS, HighValueAccountStatistics_RequiresVotingKeyForVotingEligibleAccounts) {
		// Arrange:
		auto deltaAction = [](const auto& addresses, auto& delta) {
			// - add 3/4 accounts with sufficient balance (uncommitted) [7 match]
			auto uncommittedAddresses = AddAccountsWithBalances(*delta, {
				Amount(1'100'000), Amount(900'000), Amount(1'000'000), Amount(1'500'000)
			});

			// - modify two [7 match]
			delta->find(addresses[1]).get().Balances.credit(Harvesting_Mosaic_Id, Amount(500'000));
			delta->find(addresses[4]).get().Balances.debit(Harvesting_Mosaic_Id, Amount(200'001));

			// - delete two [5 match]
			delta->queueRemove(addresses[2], Height(1));
			delta->queueRemove(uncommittedAddresses[0], Height(1));
			delta->commitRemovals();

			// - create a second entry for a voting account [5 match]
			delta->find(addresses[5]).get().Balances.credit(Harvesting_Mosaic_Id, Amount(1));
			delta->updateHighValueAccounts(Height(1));

			// - remove a voting key [5 match]
			// - importantly, the account is NOT pruned because earlier entry is voting eligible
			{
				auto accountIter = delta->find(addresses[5]);
				auto& votingPublicKeys = accountIter.get().SupplementalPublicKeys.voting();
				votingPublicKeys.remove(votingPublicKeys.get(0));
			}

			// Act:
			delta->updateHighValueAccounts(Height(2));
			auto statistics = ReadOnlyAccountStateCache(*delta).highValueAccountStatistics();

			// Assert:
			EXPECT_EQ(3u, statistics.VotingEligibleAccountsCount);
			EXPECT_EQ(5u, statistics.HarvestingEligibleAccountsCount);
			EXPECT_EQ(Amount(4'000'000), statistics.TotalVotingBalance);
		};
		auto viewAction = [](const auto&, const auto& view) {
			// Act:
			auto statistics = ReadOnlyAccountStateCache(*view).highValueAccountStatistics();

			// Assert:
			EXPECT_EQ(3u, statistics.VotingEligibleAccountsCount);
			EXPECT_EQ(4u, statistics.HarvestingEligibleAccountsCount);
			EXPECT_EQ(Amount(6'300'000), statistics.TotalVotingBalance);
		};

		// - add 4/6 accounts with sufficient balance [4 match]
		auto balances = std::vector<Amount>{
			Amount(1'100'000), Amount(900'000), Amount(1'000'000), Amount(800'000), Amount(1'200'000), Amount(4'000'000)
		};
		RunHighValueAddressesTest(balances, deltaAction, viewAction);
	}

	// endregion
}}
