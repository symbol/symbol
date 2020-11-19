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

#include "catapult/state/AccountBalances.h"
#include "tests/test/core/TransactionTestUtils.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountBalancesTests

	namespace {
		constexpr MosaicId Test_Mosaic_Id1 = MosaicId(12345);
		constexpr MosaicId Test_Mosaic_Id2 = MosaicId(54321);
		constexpr MosaicId Test_Mosaic_Id3 = MosaicId(99999);
	}

	// region construction + assignment

	TEST(TEST_CLASS, CanCreateEmptyAccountBalances) {
		// Arrange:
		AccountBalances balances;

		// Act:
		auto amount1 = balances.get(Test_Mosaic_Id1);
		auto amount2 = balances.get(Test_Mosaic_Id2);

		// Assert:
		EXPECT_EQ(0u, balances.size());
		EXPECT_EQ(Amount(0), amount1);
		EXPECT_EQ(Amount(0), amount2);

		EXPECT_EQ(MosaicId(), balances.optimizedMosaicId());
	}

	namespace {
		AccountBalances CreateBalancesForConstructionTests(MosaicId optimizedMosaicId) {
			AccountBalances balances;
			balances.credit(Test_Mosaic_Id2, Amount(777));
			balances.credit(Test_Mosaic_Id1, Amount(1000));
			balances.optimize(optimizedMosaicId);
			return balances;
		}

		void AssertCopied(const AccountBalances& balances, const AccountBalances& balancesCopy, MosaicId optimizedMosaicId) {
			// Assert: the copy is detached from the original
			EXPECT_EQ(Amount(777), balances.get(Test_Mosaic_Id2));
			EXPECT_EQ(Amount(1000), balances.get(Test_Mosaic_Id1));

			EXPECT_EQ(Amount(777), balancesCopy.get(Test_Mosaic_Id2));
			EXPECT_EQ(Amount(1500), balancesCopy.get(Test_Mosaic_Id1));

			// - optimization is preserved
			EXPECT_EQ(optimizedMosaicId, balancesCopy.optimizedMosaicId());
		}

		void AssertMoved(const AccountBalances& balances, const AccountBalances& balancesMoved, MosaicId optimizedMosaicId) {
			// Assert: the original values are moved into the copy (move does not clear first mosaic)
			if (Test_Mosaic_Id2 == optimizedMosaicId) {
				EXPECT_EQ(Amount(777), balances.get(Test_Mosaic_Id2));
				EXPECT_EQ(Amount(0), balances.get(Test_Mosaic_Id1));
			} else {
				EXPECT_EQ(Amount(0), balances.get(Test_Mosaic_Id2));
				EXPECT_EQ(Amount(1000), balances.get(Test_Mosaic_Id1));
			}

			EXPECT_EQ(Amount(777), balancesMoved.get(Test_Mosaic_Id2));
			EXPECT_EQ(Amount(1000), balancesMoved.get(Test_Mosaic_Id1));

			// - optimization is preserved
			EXPECT_EQ(optimizedMosaicId, balancesMoved.optimizedMosaicId());
		}

		void AssertCanCopyConstructAccountBalances(MosaicId optimizedMosaicId) {
			// Arrange:
			auto balances = CreateBalancesForConstructionTests(optimizedMosaicId);

			// Act:
			AccountBalances balancesCopy(balances);
			balancesCopy.credit(Test_Mosaic_Id1, Amount(500));

			// Assert:
			AssertCopied(balances, balancesCopy, optimizedMosaicId);
		}

		void AssertCanMoveConstructAccountBalances(MosaicId optimizedMosaicId) {
			// Arrange:
			auto balances = CreateBalancesForConstructionTests(optimizedMosaicId);

			// Act:
			AccountBalances balancesMoved(std::move(balances));

			// Assert:
			AssertMoved(balances, balancesMoved, optimizedMosaicId);
		}

		void AssertCanAssignAccountBalances(MosaicId optimizedMosaicId) {
			// Arrange:
			auto balances = CreateBalancesForConstructionTests(optimizedMosaicId);

			// Act:
			AccountBalances balancesCopy;
			const auto& assignResult = balancesCopy = balances;
			balancesCopy.credit(Test_Mosaic_Id1, Amount(500));

			// Assert:
			EXPECT_EQ(&balancesCopy, &assignResult);
			AssertCopied(balances, balancesCopy, optimizedMosaicId);
		}

		void AssertCanMoveAssignAccountBalances(MosaicId optimizedMosaicId) {
			// Arrange:
			auto balances = CreateBalancesForConstructionTests(optimizedMosaicId);

			// Act:
			AccountBalances balancesMoved;
			const auto& assignResult = balancesMoved = std::move(balances);

			// Assert:
			EXPECT_EQ(&balancesMoved, &assignResult);
			AssertMoved(balances, balancesMoved, optimizedMosaicId);
		}
	}

	TEST(TEST_CLASS, CanCopyConstructAccountBalances) {
		AssertCanCopyConstructAccountBalances(Test_Mosaic_Id2);
	}

	TEST(TEST_CLASS, CanCopyConstructAccountBalances_NoOptimization) {
		AssertCanCopyConstructAccountBalances(MosaicId());
	}

	TEST(TEST_CLASS, CanMoveConstructAccountBalances) {
		AssertCanMoveConstructAccountBalances(Test_Mosaic_Id2);
	}

	TEST(TEST_CLASS, CanMoveConstructAccountBalances_NoOptimization) {
		AssertCanMoveConstructAccountBalances(MosaicId());
	}

	TEST(TEST_CLASS, CanAssignAccountBalances) {
		AssertCanAssignAccountBalances(Test_Mosaic_Id2);
	}

	TEST(TEST_CLASS, CanAssignAccountBalances_NoOptimization) {
		AssertCanAssignAccountBalances(MosaicId());
	}

	TEST(TEST_CLASS, CanMoveAssignAccountBalances) {
		AssertCanMoveAssignAccountBalances(Test_Mosaic_Id2);
	}

	TEST(TEST_CLASS, CanMoveAssignAccountBalances_NoOptimization) {
		AssertCanMoveAssignAccountBalances(MosaicId());
	}

	// endregion

	// region credit

	TEST(TEST_CLASS, CreditDoesNotAddZeroBalance) {
		// Arrange:
		AccountBalances balances;

		// Act:
		balances.credit(Test_Mosaic_Id1, Amount(0));

		// Assert:
		EXPECT_EQ(0u, balances.size());
		EXPECT_EQ(Amount(0), balances.get(Test_Mosaic_Id1));
	}

	TEST(TEST_CLASS, CreditIncreasesAmountStored) {
		// Arrange:
		AccountBalances balances;

		// Act:
		balances.credit(Test_Mosaic_Id1, Amount(12345));

		// Assert:
		EXPECT_EQ(1u, balances.size());
		EXPECT_EQ(Amount(12345), balances.get(Test_Mosaic_Id1));
	}

	TEST(TEST_CLASS, InterleavingCreditsYieldCorrectState) {
		// Arrange:
		AccountBalances balances;

		// Act:
		balances.credit(Test_Mosaic_Id1, Amount(12345));
		balances.credit(Test_Mosaic_Id2, Amount(3456));
		balances.credit(Test_Mosaic_Id1, Amount(54321));

		// Assert:
		EXPECT_EQ(2u, balances.size());
		EXPECT_EQ(Amount(12345 + 54321), balances.get(Test_Mosaic_Id1));
		EXPECT_EQ(Amount(3456), balances.get(Test_Mosaic_Id2));
	}

	// endregion

	// region debit

	TEST(TEST_CLASS, CanDebitZeroFromZeroBalance) {
		// Arrange:
		AccountBalances balances;

		// Act:
		balances.debit(Test_Mosaic_Id1, Amount(0));

		// Assert:
		EXPECT_EQ(0u, balances.size());
		EXPECT_EQ(Amount(0), balances.get(Test_Mosaic_Id1));
	}

	TEST(TEST_CLASS, DebitDecreasesAmountStored) {
		// Arrange:
		AccountBalances balances;
		balances.credit(Test_Mosaic_Id1, Amount(12345));

		// Act:
		balances.debit(Test_Mosaic_Id1, Amount(222));

		// Assert:
		EXPECT_EQ(1u, balances.size());
		EXPECT_EQ(Amount(12345 - 222), balances.get(Test_Mosaic_Id1));
	}

	TEST(TEST_CLASS, FullDebitRemovesMosaicFromCache) {
		// Arrange:
		AccountBalances balances;
		balances.credit(Test_Mosaic_Id1, Amount(12345));

		// Act:
		balances.debit(Test_Mosaic_Id1, Amount(12345));
		auto amount = balances.get(Test_Mosaic_Id1);

		// Assert:
		EXPECT_EQ(0u, balances.size());
		EXPECT_EQ(Amount(0), amount);
	}

	TEST(TEST_CLASS, InterleavingDebitsYieldCorrectState) {
		// Arrange:
		AccountBalances balances;
		balances.credit(Test_Mosaic_Id1, Amount(12345));
		balances.credit(Test_Mosaic_Id2, Amount(3456));

		// Act:
		balances.debit(Test_Mosaic_Id1, Amount(222));
		balances.debit(Test_Mosaic_Id2, Amount(1111));
		balances.debit(Test_Mosaic_Id1, Amount(111));

		// Assert:
		EXPECT_EQ(2u, balances.size());
		EXPECT_EQ(Amount(12345 - 222 - 111), balances.get(Test_Mosaic_Id1));
		EXPECT_EQ(Amount(3456 - 1111), balances.get(Test_Mosaic_Id2));
	}

	TEST(TEST_CLASS, DebitDoesNotAllowUnderflowOfNonzeroBalance) {
		// Arrange:
		AccountBalances balances;
		balances.credit(Test_Mosaic_Id1, Amount(12345));

		// Act + Assert:
		EXPECT_THROW(balances.debit(Test_Mosaic_Id1, Amount(12346)), catapult_runtime_error);

		// Assert:
		EXPECT_EQ(1u, balances.size());
		EXPECT_EQ(Amount(12345), balances.get(Test_Mosaic_Id1));
	}

	TEST(TEST_CLASS, DebitDoesNotAllowUnderflowOfZeroBalance) {
		// Arrange:
		AccountBalances balances;

		// Act + Assert:
		EXPECT_THROW(balances.debit(Test_Mosaic_Id1, Amount(222)), catapult_runtime_error);

		// Assert:
		EXPECT_EQ(0u, balances.size());
		EXPECT_EQ(Amount(0), balances.get(Test_Mosaic_Id1));
	}

	// endregion

	// region credit + debit

	TEST(TEST_CLASS, InterleavingDebitsAndCreditsYieldCorrectState) {
		// Arrange:
		AccountBalances balances;
		balances.credit(Test_Mosaic_Id1, Amount(12345));
		balances.credit(Test_Mosaic_Id2, Amount(3456));

		// Act:
		balances.debit(Test_Mosaic_Id2, Amount(1111));
		balances.credit(Test_Mosaic_Id1, Amount(1111));
		balances.credit(Test_Mosaic_Id3, Amount(0)); // no op
		balances.debit(Test_Mosaic_Id1, Amount(2345));
		balances.debit(Test_Mosaic_Id3, Amount(0)); // no op
		balances.credit(Test_Mosaic_Id2, Amount(5432));

		// Assert:
		EXPECT_EQ(2u, balances.size());
		EXPECT_EQ(Amount(12345 + 1111 - 2345), balances.get(Test_Mosaic_Id1));
		EXPECT_EQ(Amount(3456 - 1111 + 5432), balances.get(Test_Mosaic_Id2));
	}

	TEST(TEST_CLASS, ChainedInterleavingDebitsAndCreditsYieldCorrectState) {
		// Arrange:
		AccountBalances balances;
		balances
			.credit(Test_Mosaic_Id1, Amount(12345))
			.credit(Test_Mosaic_Id2, Amount(3456));

		// Act:
		balances
			.debit(Test_Mosaic_Id2, Amount(1111))
			.credit(Test_Mosaic_Id1, Amount(1111))
			.credit(Test_Mosaic_Id3, Amount(0)) // no op
			.debit(Test_Mosaic_Id1, Amount(2345))
			.debit(Test_Mosaic_Id3, Amount(0)) // no op
			.credit(Test_Mosaic_Id2, Amount(5432));

		// Assert:
		EXPECT_EQ(2u, balances.size());
		EXPECT_EQ(Amount(12345 + 1111 - 2345), balances.get(Test_Mosaic_Id1));
		EXPECT_EQ(Amount(3456 - 1111 + 5432), balances.get(Test_Mosaic_Id2));
	}

	// endregion

	// region optimize

	TEST(TEST_CLASS, CanOptimizeMosaicStorage) {
		// Arrange:
		AccountBalances balances;
		balances
			.credit(Test_Mosaic_Id1, Amount(12345))
			.credit(Test_Mosaic_Id3, Amount(2244))
			.credit(Test_Mosaic_Id2, Amount(3456));

		// Sanity:
		EXPECT_EQ(Test_Mosaic_Id1, balances.begin()->first);

		// Act:
		balances.optimize(Test_Mosaic_Id2);

		// Assert:
		EXPECT_EQ(Test_Mosaic_Id2, balances.optimizedMosaicId());
		EXPECT_EQ(Test_Mosaic_Id2, balances.begin()->first);
	}

	// endregion

	// region iteration

	TEST(TEST_CLASS, CanIterateOverAllBalances) {
		// Arrange:
		AccountBalances balances;
		balances
			.credit(Test_Mosaic_Id1, Amount(12345))
			.credit(Test_Mosaic_Id3, Amount(0))
			.credit(Test_Mosaic_Id2, Amount(3456));

		// Act:
		auto numBalances = 0u;
		std::map<MosaicId, Amount> iteratedBalances;
		for (const auto& pair : balances) {
			iteratedBalances.emplace(pair);
			++numBalances;
		}

		// Assert:
		EXPECT_EQ(2u, numBalances);
		EXPECT_EQ(2u, iteratedBalances.size());
		EXPECT_EQ(Amount(12345), iteratedBalances[Test_Mosaic_Id1]);
		EXPECT_EQ(Amount(3456), iteratedBalances[Test_Mosaic_Id2]);
	}

	// endregion
}}
