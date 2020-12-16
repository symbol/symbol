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

#include "src/importance/ImportanceCalculator.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace importance {

#define TEST_CLASS RestoreImportanceCalculatorTests

	namespace {
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);
		constexpr auto Default_Balance = Amount(100);
		constexpr auto No_Op = [](const state::AccountActivityBuckets::HeightDetachedActivityBucket&) {};

		struct TestHeights {
			static constexpr auto Zero = model::ImportanceHeight(0);
			static constexpr auto One = model::ImportanceHeight(100);
			static constexpr auto Two = model::ImportanceHeight(200);
			static constexpr auto Three = model::ImportanceHeight(300);
		};

		auto& AddRandomAccount(cache::AccountStateCacheDelta& delta) {
			auto address = test::GenerateRandomByteArray<Address>();
			delta.addAccount(address, Height(1));
			auto& accountState = delta.find(address).get();
			accountState.Balances.credit(Harvesting_Mosaic_Id, Default_Balance);
			return accountState;
		}

		void AssertBucket(const state::AccountActivityBuckets& buckets, model::ImportanceHeight expectedStartHeight) {
			EXPECT_EQ(expectedStartHeight, buckets.begin()->StartHeight);
		}

		void AssertImportance(const state::AccountImportanceSnapshots& snapshots, Importance importance, model::ImportanceHeight height) {
			EXPECT_EQ(importance, snapshots.current());
			EXPECT_EQ(height, snapshots.height());
		}

		template<typename TAction>
		void RunTestWithDelta(TAction action) {
			// Arrange:
			cache::AccountStateCache cache(cache::CacheConfiguration(), test::CreateDefaultAccountStateCacheOptions());
			auto cacheDelta = cache.createDelta();

			// Act:
			return action(*cacheDelta);
		}

		void Recalculate(model::ImportanceHeight importanceHeight, cache::AccountStateCacheDelta& delta) {
			delta.updateHighValueAccounts(Height(1));
			CreateRestoreImportanceCalculator()->recalculate(ImportanceRollbackMode::Disabled, importanceHeight, delta);
		}
	}

	// region failure

	TEST(TEST_CLASS, FailureWhenAccountDoesNotHaveAnyImportanceSet) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ActivityBuckets.update(TestHeights::One, No_Op);

			// Act:
			EXPECT_THROW(Recalculate(TestHeights::One, delta), catapult_out_of_range);
		});
	}

	TEST(TEST_CLASS, FailureWhenAccountDoesNotHaveAnyActivityBucketSet) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ImportanceSnapshots.set(Importance(777), TestHeights::One);

			// Act:
			EXPECT_THROW(Recalculate(TestHeights::One, delta), catapult_out_of_range);
		});
	}

	// endregion

	// region importance is popped

	TEST(TEST_CLASS, ImportancePoppedWhenAccountHasImportanceSetAtHeightGreaterThanRestoreHeightAndHasImportanceAtRestoreHeight) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ImportanceSnapshots.set(Importance(777), TestHeights::One);
			accountState.ImportanceSnapshots.set(Importance(888), TestHeights::Two);
			accountState.ImportanceSnapshots.set(Importance(999), TestHeights::Three);
			accountState.ActivityBuckets.update(TestHeights::One, No_Op);
			accountState.ActivityBuckets.update(TestHeights::Two, No_Op);
			accountState.ActivityBuckets.update(TestHeights::Three, No_Op);

			// Act:
			Recalculate(TestHeights::Two, delta);

			// Assert: restored (TH.3 > TH.2) to TH.2
			AssertImportance(accountState.ImportanceSnapshots, Importance(888), TestHeights::Two);
			AssertBucket(accountState.ActivityBuckets, TestHeights::Two);
		});
	}

	TEST(TEST_CLASS, ImportancePoppedWhenAccountHasImportanceSetAtHeightGreaterThanRestoreHeightAndDoesNotHaveImportanceAtRestoreHeight) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ImportanceSnapshots.set(Importance(777), TestHeights::One);
			accountState.ImportanceSnapshots.set(Importance(999), TestHeights::Three);
			accountState.ActivityBuckets.update(TestHeights::One, No_Op);
			accountState.ActivityBuckets.update(TestHeights::Three, No_Op);

			// Act:
			Recalculate(TestHeights::Two, delta);

			// Assert: restored (TH.3 > TH.2) to TH.1
			AssertImportance(accountState.ImportanceSnapshots, Importance(777), TestHeights::One);
			AssertBucket(accountState.ActivityBuckets, TestHeights::One);
		});
	}

	TEST(TEST_CLASS, ImportancePoppedAtMostOncePerCalculation) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ImportanceSnapshots.set(Importance(777), TestHeights::One);
			accountState.ImportanceSnapshots.set(Importance(888), TestHeights::Two);
			accountState.ImportanceSnapshots.set(Importance(999), TestHeights::Three);
			accountState.ActivityBuckets.update(TestHeights::One, No_Op);
			accountState.ActivityBuckets.update(TestHeights::Two, No_Op);
			accountState.ActivityBuckets.update(TestHeights::Three, No_Op);

			// Act: restore height
			Recalculate(TestHeights::One, delta);

			// Assert: restored (TH.3 > TH.2 > TH.1) to TH.2
			//         (this is an edge case scenario that cannot happen in production because rollbacks can't be skipped)
			AssertImportance(accountState.ImportanceSnapshots, Importance(888), TestHeights::Two);
			AssertBucket(accountState.ActivityBuckets, TestHeights::Two);
		});
	}

	TEST(TEST_CLASS, ImportanceZeroedWhenLastImportanceIsPopped) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ImportanceSnapshots.set(Importance(987), TestHeights::Two);
			accountState.ActivityBuckets.update(TestHeights::Two, No_Op);

			// Act:
			Recalculate(TestHeights::One, delta);

			// Assert: restored (TH.2 > TH.1) to 0
			AssertImportance(accountState.ImportanceSnapshots, Importance(), TestHeights::Zero);
			AssertBucket(accountState.ActivityBuckets, TestHeights::Zero);
		});
	}

	// endregion

	// region multiple accounts

	namespace {
		std::vector<Address> SeedFourAccounts(cache::AccountStateCacheDelta& delta) {
			auto& accountState1 = AddRandomAccount(delta);
			accountState1.ImportanceSnapshots.set(Importance(1), TestHeights::One);
			accountState1.ImportanceSnapshots.set(Importance(2), TestHeights::Two);
			accountState1.ImportanceSnapshots.set(Importance(3), TestHeights::Three);
			accountState1.ActivityBuckets.update(TestHeights::One, No_Op);
			accountState1.ActivityBuckets.update(TestHeights::Two, No_Op);
			accountState1.ActivityBuckets.update(TestHeights::Three, No_Op);

			auto& accountState2 = AddRandomAccount(delta);
			accountState2.ImportanceSnapshots.set(Importance(9), TestHeights::Three);
			accountState2.ActivityBuckets.update(TestHeights::Three, No_Op);

			auto& accountState3 = AddRandomAccount(delta);
			accountState3.ImportanceSnapshots.set(Importance(1), TestHeights::One);
			accountState3.ImportanceSnapshots.set(Importance(3), TestHeights::Three);
			accountState3.ActivityBuckets.update(TestHeights::One, No_Op);
			accountState3.ActivityBuckets.update(TestHeights::Three, No_Op);

			auto& accountState4 = AddRandomAccount(delta);
			accountState4.ImportanceSnapshots.set(Importance(4), TestHeights::One);
			accountState4.ImportanceSnapshots.set(Importance(5), TestHeights::Two);
			accountState4.ImportanceSnapshots.set(Importance(6), TestHeights::Three);
			accountState4.ActivityBuckets.update(TestHeights::One, No_Op);
			accountState4.ActivityBuckets.update(TestHeights::Two, No_Op);
			accountState4.ActivityBuckets.update(TestHeights::Three, No_Op);

			return { accountState1.Address, accountState2.Address, accountState3.Address, accountState4.Address };
		}
	}

	TEST(TEST_CLASS, ImportancesAreRestoredForMultipleAccounts) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto addresses = SeedFourAccounts(delta);

			// Act:
			Recalculate(TestHeights::Two, delta);

			// Assert:
			EXPECT_EQ(4u, delta.size());
			AssertImportance(delta.find(addresses[0]).get().ImportanceSnapshots, Importance(2), TestHeights::Two);
			AssertImportance(delta.find(addresses[1]).get().ImportanceSnapshots, Importance(), TestHeights::Zero);
			AssertImportance(delta.find(addresses[2]).get().ImportanceSnapshots, Importance(1), TestHeights::One);
			AssertImportance(delta.find(addresses[3]).get().ImportanceSnapshots, Importance(5), TestHeights::Two);
			AssertBucket(delta.find(addresses[0]).get().ActivityBuckets, TestHeights::Two);
			AssertBucket(delta.find(addresses[1]).get().ActivityBuckets, TestHeights::Zero);
			AssertBucket(delta.find(addresses[2]).get().ActivityBuckets, TestHeights::One);
			AssertBucket(delta.find(addresses[3]).get().ActivityBuckets, TestHeights::Two);
		});
	}

	TEST(TEST_CLASS, ImportancesAreRestoredForMultipleAccountsWithRemovals) {
		// Arrange:
		auto options = test::CreateDefaultAccountStateCacheOptions();
		options.HarvestingMosaicId = Harvesting_Mosaic_Id;
		options.MinHarvesterBalance = Amount(1);
		cache::AccountStateCache cache(cache::CacheConfiguration(), options);

		std::vector<Address> addresses;
		{
			auto lockedDelta = cache.createDelta();
			auto& delta = *lockedDelta;
			addresses = SeedFourAccounts(delta);
			delta.updateHighValueAccounts(Height(1));
			cache.commit();
		}

		// - transition one account to unimportant and delete another
		auto lockedDelta = cache.createDelta();
		auto& delta = *lockedDelta;
		delta.queueRemove(addresses[1], Height(1));
		delta.find(addresses[2]).get().Balances.debit(Harvesting_Mosaic_Id, Default_Balance);
		delta.commitRemovals();
		delta.updateHighValueAccounts(Height(1));

		// Act:
		Recalculate(TestHeights::Two, delta);

		// Assert:
		EXPECT_EQ(3u, delta.size());
		AssertImportance(delta.find(addresses[0]).get().ImportanceSnapshots, Importance(2), TestHeights::Two);
		AssertImportance(delta.find(addresses[2]).get().ImportanceSnapshots, Importance(1), TestHeights::One);
		AssertImportance(delta.find(addresses[3]).get().ImportanceSnapshots, Importance(5), TestHeights::Two);
		AssertBucket(delta.find(addresses[0]).get().ActivityBuckets, TestHeights::Two);
		AssertBucket(delta.find(addresses[2]).get().ActivityBuckets, TestHeights::One);
		AssertBucket(delta.find(addresses[3]).get().ActivityBuckets, TestHeights::Two);
	}

	// endregion
}}
