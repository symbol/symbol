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

#include "src/observers/ImportanceCalculator.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS RestoreImportanceCalculatorTests

	namespace {
		struct TestHeights {
			static constexpr auto Zero = model::ImportanceHeight(0);
			static constexpr auto One = model::ImportanceHeight(100);
			static constexpr auto Two = model::ImportanceHeight(200);
			static constexpr auto Three = model::ImportanceHeight(300);
		};

		auto& AddRandomAccount(cache::AccountStateCacheDelta& delta) {
			auto key = test::GenerateRandomData<Key_Size>();
			delta.addAccount(key, Height(1));
			return delta.find(key).get();
		}

		void AssertImportance(const state::AccountImportance& accountImportance, Importance importance, model::ImportanceHeight height) {
			EXPECT_EQ(importance, accountImportance.current());
			EXPECT_EQ(height, accountImportance.height());
		}

		template<typename TAction>
		void RunTestWithDelta(TAction action) {
			// Arrange:
			auto networkIdentifier = model::NetworkIdentifier::Mijin_Test;
			cache::AccountStateCacheTypes::Options options{ networkIdentifier, 123, Amount(0), MosaicId(1111), MosaicId(2222) };
			cache::AccountStateCache cache(cache::CacheConfiguration(), options);
			auto cacheDelta = cache.createDelta();

			// Act:
			return action(*cacheDelta);
		}
	}

	// region no importance change

	TEST(TEST_CLASS, NoImportanceChangeWhenAccountDoesNotHaveAnyImportanceSet) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			const auto& accountState = AddRandomAccount(delta);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(TestHeights::Two, delta);

			// Assert: nothing to restore
			AssertImportance(accountState.ImportanceInfo, Importance(), TestHeights::Zero);
		});
	}

	TEST(TEST_CLASS, NoImportanceChangeWhenAccountHasImportanceSetAtHeightLessThanRestoreHeight) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ImportanceInfo.set(Importance(987), TestHeights::Two);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(TestHeights::Three, delta);

			// Assert: nothing to restore (TH.2 < TH.3)
			AssertImportance(accountState.ImportanceInfo, Importance(987), TestHeights::Two);
		});
	}

	TEST(TEST_CLASS, NoImportanceChangeWhenAccountHasImportanceSetAtRestoreHeight) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ImportanceInfo.set(Importance(987), TestHeights::Two);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(TestHeights::Two, delta);

			// Assert: nothing to restore (TH.2 == TH.2)
			AssertImportance(accountState.ImportanceInfo, Importance(987), TestHeights::Two);
		});
	}

	// endregion

	// region importance is popped

	TEST(TEST_CLASS, ImportancePoppedWhenAccountHasImportanceSetAtHeightGreaterThanRestoreHeightAndHasImportanceAtRestoreHeight) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ImportanceInfo.set(Importance(777), TestHeights::One);
			accountState.ImportanceInfo.set(Importance(888), TestHeights::Two);
			accountState.ImportanceInfo.set(Importance(999), TestHeights::Three);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(TestHeights::Two, delta);

			// Assert: restored (TH.3 > TH.2) to TH.2
			AssertImportance(accountState.ImportanceInfo, Importance(888), TestHeights::Two);
		});
	}

	TEST(TEST_CLASS, ImportancePoppedWhenAccountHasImportanceSetAtHeightGreaterThanRestoreHeightAndDoesNotHaveImportanceAtRestoreHeight) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ImportanceInfo.set(Importance(777), TestHeights::One);
			accountState.ImportanceInfo.set(Importance(999), TestHeights::Three);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(TestHeights::Two, delta);

			// Assert: restored (TH.3 > TH.2) to TH.1
			AssertImportance(accountState.ImportanceInfo, Importance(777), TestHeights::One);
		});
	}

	TEST(TEST_CLASS, ImportancePoppedAtMostOncePerCalculation) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ImportanceInfo.set(Importance(777), TestHeights::One);
			accountState.ImportanceInfo.set(Importance(888), TestHeights::Two);
			accountState.ImportanceInfo.set(Importance(999), TestHeights::Three);

			// Act: restore height
			CreateRestoreImportanceCalculator()->recalculate(TestHeights::One, delta);

			// Assert: restored (TH.3 > TH.2 > TH.1) to TH.2
			//         (this is an edge case scenario that cannot happen in production because rollbacks can't be skipped)
			AssertImportance(accountState.ImportanceInfo, Importance(888), TestHeights::Two);
		});
	}

	TEST(TEST_CLASS, ImportanceZeroedIfLastImportanceIsPopped) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState = AddRandomAccount(delta);
			accountState.ImportanceInfo.set(Importance(987), TestHeights::Two);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(TestHeights::One, delta);

			// Assert: restored (TH.2 > TH.1) to 0
			AssertImportance(accountState.ImportanceInfo, Importance(), TestHeights::Zero);
		});
	}

	// endregion

	// region multiple accounts

	TEST(TEST_CLASS, ImportancesAreRestoredForMultipleAccounts) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto& accountState1 = AddRandomAccount(delta);
			accountState1.ImportanceInfo.set(Importance(1), TestHeights::One);
			accountState1.ImportanceInfo.set(Importance(2), TestHeights::Two);
			accountState1.ImportanceInfo.set(Importance(3), TestHeights::Three);

			const auto& accountState2 = AddRandomAccount(delta);

			auto& accountState3 = AddRandomAccount(delta);
			accountState3.ImportanceInfo.set(Importance(1), TestHeights::One);
			accountState3.ImportanceInfo.set(Importance(3), TestHeights::Three);

			auto& accountState4 = AddRandomAccount(delta);
			accountState4.ImportanceInfo.set(Importance(4), TestHeights::One);
			accountState4.ImportanceInfo.set(Importance(5), TestHeights::Two);
			accountState4.ImportanceInfo.set(Importance(6), TestHeights::Three);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(TestHeights::Two, delta);

			// Assert:
			AssertImportance(accountState1.ImportanceInfo, Importance(2), TestHeights::Two);
			AssertImportance(accountState2.ImportanceInfo, Importance(), TestHeights::Zero);
			AssertImportance(accountState3.ImportanceInfo, Importance(1), TestHeights::One);
			AssertImportance(accountState4.ImportanceInfo, Importance(5), TestHeights::Two);
		});
	}

	// endregion
}}
