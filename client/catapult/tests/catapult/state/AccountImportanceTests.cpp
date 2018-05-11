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

#include "catapult/state/AccountImportance.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountImportanceTests

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
				const auto message =
						"importance height " + std::to_string(pair.Height.unwrap())
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

		struct WithNoImportancesTraits {
			static void Prepare(AccountImportance&) {
			}

			static void Assert(const AccountImportance& accountImportance) {
				AssertCurrentImportance(accountImportance, Importance(0), model::ImportanceHeight(0));
			}
		};

		struct WithImportancesTraits {
			static void Prepare(AccountImportance& accountImportance) {
				accountImportance.set(Importance(123), model::ImportanceHeight(234));
			}

			static void Assert(const AccountImportance& accountImportance) {
				AssertCurrentImportance(accountImportance, Importance(123), model::ImportanceHeight(234));
			}
		};
	}

#define IMPORTANCES_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_WithNoImportances) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<WithNoImportancesTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_WithImportances) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<WithImportancesTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region construction + assignment

	TEST(TEST_CLASS, CanCreateAccountImportance) {
		// Act:
		AccountImportance accountImportance;

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(0), model::ImportanceHeight(0));
		AssertHistoricalValues(accountImportance, { { std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	IMPORTANCES_BASED_TEST(CanCopyConstructAccountImportance) {
		// Arrange:
		AccountImportance accountImportance;
		TTraits::Prepare(accountImportance);

		// Act:
		AccountImportance accountImportanceCopy(accountImportance);
		accountImportanceCopy.set(Importance(222), model::ImportanceHeight(444));

		// Assert: the copy is detached from the original
		TTraits::Assert(accountImportance);
		AssertCurrentImportance(accountImportanceCopy, Importance(222), model::ImportanceHeight(444));
	}

	IMPORTANCES_BASED_TEST(CanMoveConstructAccountImportance) {
		// Arrange:
		AccountImportance accountImportance;
		TTraits::Prepare(accountImportance);

		// Act:
		AccountImportance accountImportanceMoved(std::move(accountImportance));

		// Assert: the original values are moved into the copy
		AssertCurrentImportance(accountImportance, Importance(0), model::ImportanceHeight(0));
		TTraits::Assert(accountImportanceMoved);
	}

	IMPORTANCES_BASED_TEST(CanAssignAccountImportance) {
		// Arrange:
		AccountImportance accountImportance;
		TTraits::Prepare(accountImportance);

		// Act:
		AccountImportance accountImportanceCopy;
		const auto& assignResult = accountImportanceCopy = accountImportance;
		accountImportanceCopy.set(Importance(222), model::ImportanceHeight(444));

		// Assert: the copy is detached from the original
		EXPECT_EQ(&accountImportanceCopy, &assignResult);
		TTraits::Assert(accountImportance);
		AssertCurrentImportance(accountImportanceCopy, Importance(222), model::ImportanceHeight(444));
	}

	IMPORTANCES_BASED_TEST(CanMoveAssignAccountImportance) {
		// Arrange:
		AccountImportance accountImportance;
		TTraits::Prepare(accountImportance);

		// Act:
		AccountImportance accountImportanceMoved;
		const auto& assignResult = accountImportanceMoved = std::move(accountImportance);

		// Assert: the original values are moved into the copy
		EXPECT_EQ(&accountImportanceMoved, &assignResult);
		AssertCurrentImportance(accountImportance, Importance(0), model::ImportanceHeight(0));
		TTraits::Assert(accountImportanceMoved);
	}

	// endregion

	TEST(TEST_CLASS, CanSetAccountImportance) {
		// Act:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(123), model::ImportanceHeight(234));
		AssertHistoricalValues(accountImportance, { { std::make_pair(123, 234), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CannotSetAccountImportanceAtLowerHeight) {
		// Arrange:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));

		// Act + Assert:
		EXPECT_THROW(accountImportance.set(Importance(246), model::ImportanceHeight(233)), catapult_runtime_error);
		EXPECT_THROW(accountImportance.set(Importance(246), model::ImportanceHeight(100)), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotSetAccountImportanceAtSameHeight) {
		// Arrange:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));

		// Act + Assert:
		EXPECT_THROW(accountImportance.set(Importance(246), model::ImportanceHeight(234)), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanSetAccountImportanceAtHigherHeight) {
		// Arrange:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));

		// Act:
		accountImportance.set(Importance(246), model::ImportanceHeight(235));

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(246), model::ImportanceHeight(235));
		AssertHistoricalValues(accountImportance, { { std::make_pair(246, 235), std::make_pair(123, 234), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CanSetMaxHistoricalImportances) {
		// Act:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));
		accountImportance.set(Importance(222), model::ImportanceHeight(444));
		accountImportance.set(Importance(111), model::ImportanceHeight(789));

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(111), model::ImportanceHeight(789));
		AssertHistoricalValues(accountImportance, { { std::make_pair(111, 789), std::make_pair(222, 444), std::make_pair(123, 234) } });
	}

	TEST(TEST_CLASS, CannotRetrieveUnknownHistoricalImportance) {
		// Arrange:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(123), model::ImportanceHeight(234));

		EXPECT_EQ(Importance(), accountImportance.get(model::ImportanceHeight()));
		EXPECT_EQ(Importance(), accountImportance.get(model::ImportanceHeight(233)));
		EXPECT_EQ(Importance(), accountImportance.get(model::ImportanceHeight(235)));
	}

	TEST(TEST_CLASS, SettingMoreThanHistoricalImportancesRollsOver) {
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

	TEST(TEST_CLASS, CanPopMostRecentImportance) {
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

	TEST(TEST_CLASS, CanPopAllImportances) {
		// Arrange:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));
		accountImportance.set(Importance(222), model::ImportanceHeight(444));
		accountImportance.set(Importance(111), model::ImportanceHeight(789));

		// Act:
		accountImportance.pop();
		accountImportance.pop();
		accountImportance.pop();

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(0), model::ImportanceHeight(0));
		AssertHistoricalValues(accountImportance, { { std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CanPopAllImportancesAndThenSetImportances) {
		// Arrange:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));
		accountImportance.set(Importance(222), model::ImportanceHeight(444));
		accountImportance.set(Importance(111), model::ImportanceHeight(789));

		// Act:
		accountImportance.pop();
		accountImportance.pop();
		accountImportance.pop();
		accountImportance.set(Importance(123), model::ImportanceHeight(234));

		// Assert:
		AssertCurrentImportance(accountImportance, Importance(123), model::ImportanceHeight(234));
		AssertHistoricalValues(accountImportance, { { std::make_pair(123, 234), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CannotPopMoreImportancesThanSet) {
		// Arrange:
		AccountImportance accountImportance;
		accountImportance.set(Importance(123), model::ImportanceHeight(234));

		// Act: try to pop one more importance than set
		accountImportance.pop();
		EXPECT_THROW(accountImportance.pop(), catapult_out_of_range);
	}

	// region iterators

	IMPORTANCES_BASED_TEST(CanAdvanceIteratorsPostfixOperator) {
		// Arrange:
		AccountImportance accountImportance;
		TTraits::Prepare(accountImportance);

		// Act:
		auto iter = accountImportance.begin();
		iter++;

		// Assert: only a single value is set, so advancing should always advance to a default snapshot
		EXPECT_EQ(Importance(), iter->Importance);
		EXPECT_EQ(model::ImportanceHeight(), iter->Height);
	}

	IMPORTANCES_BASED_TEST(CanAdvanceIteratorsPrefixOperator) {
		// Arrange:
		AccountImportance accountImportance;
		TTraits::Prepare(accountImportance);

		// Act:
		auto iter = ++accountImportance.begin();

		// Assert: only a single value is set, so advancing should always advance to a default snapshot
		EXPECT_EQ(Importance(), iter->Importance);
		EXPECT_EQ(model::ImportanceHeight(), iter->Height);
	}

	IMPORTANCES_BASED_TEST(CannotAdvanceIteratorsPastEnd) {
		// Arrange:
		AccountImportance accountImportance;
		TTraits::Prepare(accountImportance);

		// Act + Assert:
		EXPECT_THROW(++accountImportance.end(), catapult_out_of_range);
		EXPECT_THROW(accountImportance.end()++, catapult_out_of_range);
	}

	IMPORTANCES_BASED_TEST(CannotDereferenceIteratorsAtEnd) {
		// Arrange:
		AccountImportance accountImportance;
		TTraits::Prepare(accountImportance);

		// Act + Assert:
		EXPECT_THROW(*accountImportance.end(), catapult_out_of_range);
		EXPECT_THROW(accountImportance.end().operator->(), catapult_out_of_range);
	}

	// endregion
}}

