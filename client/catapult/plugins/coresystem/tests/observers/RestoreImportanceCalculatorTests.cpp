#include "src/observers/ImportanceCalculator.h"
#include "src/cache/AccountStateCache.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

	namespace {
		constexpr model::ImportanceHeight Test_Height_Minus_One(359);
		constexpr model::ImportanceHeight Test_Height(360);
		constexpr model::ImportanceHeight Test_Height_Plus_One(361);

		auto AddRandomAccount(cache::AccountStateCacheDelta& delta) {
			return delta.addAccount(test::GenerateRandomData<Key_Size>(), Height(Test_Height.unwrap()));
		}

		void AssertImportance(
				const state::AccountImportance& accountImportance,
				Importance importance,
				model::ImportanceHeight height) {
			EXPECT_EQ(importance, accountImportance.current());
			EXPECT_EQ(height, accountImportance.height());
		}

		template<typename TAction>
		void RunTestWithDelta(TAction action) {
			// Arrange:
			cache::AccountStateCache cache(model::NetworkIdentifier::Mijin_Test, 123);
			auto cacheDelta = cache.createDelta();

			// Act:
			return action(*cacheDelta);
		}
	}

#define NOT_LARGER_CURRENT_IMPORTANCE_HEIGHT_TEST(PREFIX) \
	TEST(RestoreImportanceCalculatorTests, PREFIX##CurrentImportanceHeightIsUnchanged)

	NOT_LARGER_CURRENT_IMPORTANCE_HEIGHT_TEST(Zero) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto pState = AddRandomAccount(delta);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(Test_Height, delta);

			// Assert: nothing to restore
			AssertImportance(pState->ImportanceInfo, Importance(), model::ImportanceHeight());
		});
	}

	NOT_LARGER_CURRENT_IMPORTANCE_HEIGHT_TEST(Smaller) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto pState = AddRandomAccount(delta);
			pState->ImportanceInfo.set(Importance(2), Test_Height);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(Test_Height_Plus_One, delta);

			// Assert: nothing to restore (TH < TH + 1)
			AssertImportance(pState->ImportanceInfo, Importance(2), Test_Height);
		});
	}

	NOT_LARGER_CURRENT_IMPORTANCE_HEIGHT_TEST(Equal) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto pState = AddRandomAccount(delta);
			pState->ImportanceInfo.set(Importance(2), Test_Height);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(Test_Height, delta);

			// Assert: nothing to restore (TH == TH)
			AssertImportance(pState->ImportanceInfo, Importance(2), Test_Height);
		});
	}

#define LARGER_CURRENT_IMPORTANCE_HEIGHT_TEST(POSTFIX) \
	TEST( \
			RestoreImportanceCalculatorTests, \
			LargerCurrentImportanceHeightIsRestoredToFirstHeightNoGreaterThanRecalculationHeight_##POSTFIX)

	LARGER_CURRENT_IMPORTANCE_HEIGHT_TEST(PreviousHeightEqual) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto pState = AddRandomAccount(delta);
			pState->ImportanceInfo.set(Importance(1), Test_Height_Minus_One);
			pState->ImportanceInfo.set(Importance(2), Test_Height);
			pState->ImportanceInfo.set(Importance(3), Test_Height_Plus_One);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(Test_Height, delta);

			// Assert: restored (TH + 1 > TH) to TH
			AssertImportance(pState->ImportanceInfo, Importance(2), Test_Height);
		});
	}

	LARGER_CURRENT_IMPORTANCE_HEIGHT_TEST(PreviousHeightLess) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto pState = AddRandomAccount(delta);
			pState->ImportanceInfo.set(Importance(1), Test_Height_Minus_One);
			pState->ImportanceInfo.set(Importance(3), Test_Height_Plus_One);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(Test_Height, delta);

			// Assert: restored (TH + 1 > TH) to TH - 1
			AssertImportance(pState->ImportanceInfo, Importance(1), Test_Height_Minus_One);
		});
	}

	LARGER_CURRENT_IMPORTANCE_HEIGHT_TEST(OldestHeightEqual) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto pState = AddRandomAccount(delta);
			pState->ImportanceInfo.set(Importance(1), Test_Height_Minus_One);
			pState->ImportanceInfo.set(Importance(2), Test_Height);
			pState->ImportanceInfo.set(Importance(3), Test_Height_Plus_One);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(Test_Height_Minus_One, delta);

			// Assert: restored (TH + 1 > TH > TH - 1) to TH - 1
			AssertImportance(pState->ImportanceInfo, Importance(1), Test_Height_Minus_One);
		});
	}

	TEST(RestoreImportanceCalculatorTests, LargerCurrentImportanceHeightIsZeroedIfAllHeightsAreGreaterThanRecalculationHeight) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto pState = AddRandomAccount(delta);
			pState->ImportanceInfo.set(Importance(2), Test_Height);
			pState->ImportanceInfo.set(Importance(3), Test_Height_Plus_One);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(Test_Height_Minus_One, delta);

			// Assert: restored (TH + 1 > TH > TH - 1) to 0
			AssertImportance(pState->ImportanceInfo, Importance(), model::ImportanceHeight());
		});
	}

	TEST(RestoreImportanceCalculatorTests, ImportancesAreRestoredForMultipleAccounts) {
		// Arrange:
		RunTestWithDelta([](auto& delta) {
			auto pState1 = AddRandomAccount(delta);
			pState1->ImportanceInfo.set(Importance(1), Test_Height_Minus_One);
			pState1->ImportanceInfo.set(Importance(2), Test_Height);
			pState1->ImportanceInfo.set(Importance(3), Test_Height_Plus_One);

			auto pState2 = AddRandomAccount(delta);

			auto pState3 = AddRandomAccount(delta);
			pState3->ImportanceInfo.set(Importance(1), Test_Height_Minus_One);
			pState3->ImportanceInfo.set(Importance(3), Test_Height_Plus_One);

			auto pState4 = AddRandomAccount(delta);
			pState4->ImportanceInfo.set(Importance(4), Test_Height_Minus_One);
			pState4->ImportanceInfo.set(Importance(5), Test_Height);
			pState4->ImportanceInfo.set(Importance(6), Test_Height_Plus_One);

			// Act:
			CreateRestoreImportanceCalculator()->recalculate(Test_Height, delta);

			// Assert:
			AssertImportance(pState1->ImportanceInfo, Importance(2), Test_Height);
			AssertImportance(pState2->ImportanceInfo, Importance(), model::ImportanceHeight());
			AssertImportance(pState3->ImportanceInfo, Importance(1), Test_Height_Minus_One);
			AssertImportance(pState4->ImportanceInfo, Importance(5), Test_Height);
		});
	}
}}
