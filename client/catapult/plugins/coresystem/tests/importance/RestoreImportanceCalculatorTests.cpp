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

#include "src/importance/ImportanceCalculator.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace importance {

#define TEST_CLASS RestoreImportanceCalculatorTests

	namespace {
		constexpr auto No_Op = [](const state::AccountActivityBuckets::HeightDetachedActivityBucket&) {};

		struct TestHeights {
			static constexpr auto Zero = model::ImportanceHeight(0);
			static constexpr auto One = model::ImportanceHeight(100);
			static constexpr auto Two = model::ImportanceHeight(200);
			static constexpr auto Three = model::ImportanceHeight(300);
		};

		auto& AddRandomAccount(cache::AccountStateCacheDelta& delta) {
			auto key = test::GenerateRandomByteArray<Key>();
			delta.addAccount(key, Height(1));
			auto& accountState = delta.find(key).get();
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
			CreateRestoreImportanceCalculator()->recalculate(importanceHeight, delta);
		}
	}

	// region no importance change

	TEST(TEST_CLASS, NoImportanceChangeWhenAccountDoesNotHaveAnyImportanceSet) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			const auto& accountState = AddRandomAccount(delta);

			// Act:
			Recalculate(TestHeights::Two, delta);

			// Assert: nothing to restore
			AssertImportance(accountState.ImportanceSnapshots, Importance(), TestHeights::Zero);
			AssertBucket(accountState.ActivityBuckets, TestHeights::Zero);
		});
	}

	TEST(TEST_CLASS, NoImportanceChangeWhenAccountHasImportanceSetAtHeightLessThanRestoreHeight) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ImportanceSnapshots.set(Importance(987), TestHeights::Two);
			accountState.ActivityBuckets.update(TestHeights::Two, No_Op);

			// Act:
			Recalculate(TestHeights::Three, delta);

			// Assert: nothing to restore (TH.2 < TH.3)
			AssertImportance(accountState.ImportanceSnapshots, Importance(987), TestHeights::Two);
			AssertBucket(accountState.ActivityBuckets, TestHeights::Two);
		});
	}

	TEST(TEST_CLASS, NoImportanceChangeWhenAccountHasImportanceSetAtRestoreHeight) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ImportanceSnapshots.set(Importance(987), TestHeights::Two);
			accountState.ActivityBuckets.update(TestHeights::Two, No_Op);

			// Act:
			Recalculate(TestHeights::Two, delta);

			// Assert: importance is not restored (TH.2 == TH.2) but bucket is removed
			AssertImportance(accountState.ImportanceSnapshots, Importance(987), TestHeights::Two);
			AssertBucket(accountState.ActivityBuckets, TestHeights::Zero);
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

	TEST(TEST_CLASS, ImportancesAreRestoredForMultipleAccounts) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState1 = AddRandomAccount(delta);
			accountState1.ImportanceSnapshots.set(Importance(1), TestHeights::One);
			accountState1.ImportanceSnapshots.set(Importance(2), TestHeights::Two);
			accountState1.ImportanceSnapshots.set(Importance(3), TestHeights::Three);
			accountState1.ActivityBuckets.update(TestHeights::One, No_Op);
			accountState1.ActivityBuckets.update(TestHeights::Two, No_Op);
			accountState1.ActivityBuckets.update(TestHeights::Three, No_Op);

			const auto& accountState2 = AddRandomAccount(delta);

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

			// Act:
			Recalculate(TestHeights::Two, delta);

			// Assert:
			AssertImportance(accountState1.ImportanceSnapshots, Importance(2), TestHeights::Two);
			AssertImportance(accountState2.ImportanceSnapshots, Importance(), TestHeights::Zero);
			AssertImportance(accountState3.ImportanceSnapshots, Importance(1), TestHeights::One);
			AssertImportance(accountState4.ImportanceSnapshots, Importance(5), TestHeights::Two);
			AssertBucket(accountState1.ActivityBuckets, TestHeights::Two);
			AssertBucket(accountState2.ActivityBuckets, TestHeights::Zero);
			AssertBucket(accountState3.ActivityBuckets, TestHeights::One);
			AssertBucket(accountState4.ActivityBuckets, TestHeights::Two);
		});
	}

	// endregion
}}
