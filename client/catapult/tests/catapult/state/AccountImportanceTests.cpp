#include "catapult/state/AccountImportance.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

	namespace {
		void AssertCurrentImportance(const AccountImportance& accountImportance, Importance importance, model::ImportanceHeight height) {
			// Assert: accessible via current functions
			EXPECT_EQ(importance, accountImportance.current());
			EXPECT_EQ(height, accountImportance.height());

			// - accessible via get at height
			EXPECT_EQ(importance, accountImportance.get(accountImportance.height()));
		}

		void AssertHistoricalValues(
				const AccountImportance& accountImportance,
				const std::array<std::pair<uint64_t, uint64_t>, Importance_History_Size>& expectedImportanceHeightPairs) {
			auto numImportances = 0u;
			for (const auto& pair : accountImportance) {
				const auto message = "importance height " + std::to_string(pair.Height.unwrap())
						+ ", index " + std::to_string(numImportances);
				const auto& expectedPair = expectedImportanceHeightPairs[numImportances];

				// Assert: pair contents
				EXPECT_EQ(model::ImportanceHeight(expectedPair.second), pair.Height) << message;
				EXPECT_EQ(Importance(expectedPair.first), pair.Importance) << message;

				// - lookup availability
				EXPECT_EQ(pair.Importance, accountImportance.get(pair.Height)) << message;
				++numImportances;
			}

			// - expected number of importances
			EXPECT_EQ(Importance_History_Size, numImportances);
		}
	}

	TEST(AccountImportanceTests, CanCreateAccountImportance) {
		// Act:
		AccountImportance accountImportance;

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(0), model::ImportanceHeight(0));
		AssertHistoricalValues(accountImportance, { { std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(AccountImportanceTests, CanSetAccountImportance) {
		// Act:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(123), model::ImportanceHeight(234));
		AssertHistoricalValues(accountImportance, { { std::make_pair(123, 234), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(AccountImportanceTests, CannotSetAccountImportanceAtLowerHeight) {
		// Arrange:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));

		// Act:
		EXPECT_THROW(accountImportance.set(Importance(246), model::ImportanceHeight(233)), catapult_runtime_error);
		EXPECT_THROW(accountImportance.set(Importance(246), model::ImportanceHeight(100)), catapult_runtime_error);
	}

	TEST(AccountImportanceTests, CannotSetAccountImportanceAtSameHeight) {
		// Arrange:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));

		// Act:
		EXPECT_THROW(accountImportance.set(Importance(246), model::ImportanceHeight(234)), catapult_runtime_error);
	}

	TEST(AccountImportanceTests, CanSetAccountImportanceAtHigherHeight) {
		// Arrange:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));

		// Act:
		accountImportance.set(Importance(246), model::ImportanceHeight(235));

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(246), model::ImportanceHeight(235));
		AssertHistoricalValues(accountImportance, { { std::make_pair(246, 235), std::make_pair(123, 234), std::make_pair(0, 0) } });
	}

	TEST(AccountImportanceTests, CanSetMaxHistoricalImportances) {
		// Act:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));
		accountImportance.set(Importance(222), model::ImportanceHeight(444));
		accountImportance.set(Importance(111), model::ImportanceHeight(789));

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(111), model::ImportanceHeight(789));
		AssertHistoricalValues(accountImportance, { { std::make_pair(111, 789), std::make_pair(222, 444), std::make_pair(123, 234) } });
	}

	TEST(AccountImportanceTests, CannotRetrieveUnknownHistoricalImportance) {
		// Arrange:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(123), model::ImportanceHeight(234));

		EXPECT_EQ(Importance(), accountImportance.get(model::ImportanceHeight()));
		EXPECT_EQ(Importance(), accountImportance.get(model::ImportanceHeight(233)));
		EXPECT_EQ(Importance(), accountImportance.get(model::ImportanceHeight(235)));
	}

	TEST(AccountImportanceTests, SettingMoreThanHistoricalImportancesRollsOver) {
		// Arrange:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));
		accountImportance.set(Importance(222), model::ImportanceHeight(444));
		accountImportance.set(Importance(111), model::ImportanceHeight(789));

		// Act:
		accountImportance.set(Importance(332), model::ImportanceHeight(888));

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(332), model::ImportanceHeight(888));
		AssertHistoricalValues(accountImportance, { { std::make_pair(332, 888), std::make_pair(111, 789), std::make_pair(222, 444) } });

		EXPECT_EQ(Importance(), accountImportance.get(model::ImportanceHeight(234)));
	}

	TEST(AccountImportanceTests, CanPopMostRecentImportance) {
		// Arrange:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));
		accountImportance.set(Importance(222), model::ImportanceHeight(444));
		accountImportance.set(Importance(111), model::ImportanceHeight(789));

		// Act:
		accountImportance.pop();

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(222), model::ImportanceHeight(444));
		AssertHistoricalValues(accountImportance, { { std::make_pair(222, 444), std::make_pair(123, 234), std::make_pair(0, 0) } });

		EXPECT_EQ(Importance(), accountImportance.get(model::ImportanceHeight(789)));
	}
}}
