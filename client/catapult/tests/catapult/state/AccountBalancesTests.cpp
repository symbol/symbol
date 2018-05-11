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

#include "catapult/state/AccountBalances.h"
#include "tests/test/core/TransactionTestUtils.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountBalancesTests

	namespace {
		constexpr MosaicId Test_Mosaic_Id = MosaicId(12345);
		constexpr MosaicId Test_Mosaic_Id2 = MosaicId(54321);
	}

	// region construction + assignment

	TEST(TEST_CLASS, CanCreateEmptyAccountBalances) {
		// Arrange:
		AccountBalances balances;

		// Act:
		auto amount1 = balances.get(Xem_Id);
		auto amount2 = balances.get(Test_Mosaic_Id);

		// Assert:
		EXPECT_EQ(0u, balances.size());
		EXPECT_EQ(Amount(0), amount1);
		EXPECT_EQ(Amount(0), amount2);
	}

	namespace {
		AccountBalances CreateBalancesForConstructionTests() {
			AccountBalances balances;
			balances.credit(Test_Mosaic_Id, Amount(777));
			balances.credit(Xem_Id, Amount(1000));
			return balances;
		}
	}

	TEST(TEST_CLASS, CanCopyConstructAccountBalances) {
		// Arrange:
		auto balances = CreateBalancesForConstructionTests();

		// Act:
		AccountBalances balancesCopy(balances);
		balancesCopy.credit(Xem_Id, Amount(500));

		// Assert: the copy is detached from the original
		EXPECT_EQ(Amount(777), balances.get(Test_Mosaic_Id));
		EXPECT_EQ(Amount(1000), balances.get(Xem_Id));

		EXPECT_EQ(Amount(777), balancesCopy.get(Test_Mosaic_Id));
		EXPECT_EQ(Amount(1500), balancesCopy.get(Xem_Id));
	}

	TEST(TEST_CLASS, CanMoveConstructAccountBalances) {
		// Arrange:
		auto balances = CreateBalancesForConstructionTests();

		// Act:
		AccountBalances balancesMoved(std::move(balances));

		// Assert: the original values are moved into the copy (move does not clear first mosaic)
		EXPECT_EQ(Amount(777), balances.get(Test_Mosaic_Id));
		EXPECT_EQ(Amount(0), balances.get(Xem_Id));

		EXPECT_EQ(Amount(777), balancesMoved.get(Test_Mosaic_Id));
		EXPECT_EQ(Amount(1000), balancesMoved.get(Xem_Id));
	}

	TEST(TEST_CLASS, CanAssignAccountBalances) {
		// Arrange:
		auto balances = CreateBalancesForConstructionTests();

		// Act:
		AccountBalances balancesCopy;
		const auto& assignResult = balancesCopy = balances;
		balancesCopy.credit(Xem_Id, Amount(500));

		// Assert: the copy is detached from the original
		EXPECT_EQ(&balancesCopy, &assignResult);
		EXPECT_EQ(Amount(777), balances.get(Test_Mosaic_Id));
		EXPECT_EQ(Amount(1000), balances.get(Xem_Id));

		EXPECT_EQ(Amount(777), balancesCopy.get(Test_Mosaic_Id));
		EXPECT_EQ(Amount(1500), balancesCopy.get(Xem_Id));
	}

	TEST(TEST_CLASS, CanMoveAssignAccountBalances) {
		// Arrange:
		auto balances = CreateBalancesForConstructionTests();

		// Act:
		AccountBalances balancesMoved;
		const auto& assignResult = balancesMoved = std::move(balances);

		// Assert: the original values are moved into the copy (move does not clear first mosaic)
		EXPECT_EQ(&balancesMoved, &assignResult);
		EXPECT_EQ(Amount(777), balances.get(Test_Mosaic_Id));
		EXPECT_EQ(Amount(0), balances.get(Xem_Id));

		EXPECT_EQ(Amount(777), balancesMoved.get(Test_Mosaic_Id));
		EXPECT_EQ(Amount(1000), balancesMoved.get(Xem_Id));
	}

	// endregion

	// region credit

	TEST(TEST_CLASS, CreditDoesNotAddZeroBalance) {
		// Arrange:
		AccountBalances balances;

		// Act:
		balances.credit(Xem_Id, Amount(0));

		// Assert:
		EXPECT_EQ(0u, balances.size());
		EXPECT_EQ(Amount(0), balances.get(Xem_Id));
	}

	TEST(TEST_CLASS, CreditIncreasesAmountStored) {
		// Arrange:
		AccountBalances balances;

		// Act:
		balances.credit(Xem_Id, Amount(12345));

		// Assert:
		EXPECT_EQ(1u, balances.size());
		EXPECT_EQ(Amount(12345), balances.get(Xem_Id));
	}

	TEST(TEST_CLASS, InterleavingCreditsYieldCorrectState) {
		// Arrange:
		AccountBalances balances;

		// Act:
		balances.credit(Xem_Id, Amount(12345));
		balances.credit(Test_Mosaic_Id, Amount(3456));
		balances.credit(Xem_Id, Amount(54321));

		// Assert:
		EXPECT_EQ(2u, balances.size());
		EXPECT_EQ(Amount(12345 + 54321), balances.get(Xem_Id));
		EXPECT_EQ(Amount(3456), balances.get(Test_Mosaic_Id));
	}

	// endregion

	// region debit

	TEST(TEST_CLASS, CanDebitZeroFromZeroBalance) {
		// Arrange:
		AccountBalances balances;

		// Act:
		balances.debit(Xem_Id, Amount(0));

		// Assert:
		EXPECT_EQ(0u, balances.size());
		EXPECT_EQ(Amount(0), balances.get(Xem_Id));
	}

	TEST(TEST_CLASS, DebitDecreasesAmountStored) {
		// Arrange:
		AccountBalances balances;
		balances.credit(Xem_Id, Amount(12345));

		// Act:
		balances.debit(Xem_Id, Amount(222));

		// Assert:
		EXPECT_EQ(1u, balances.size());
		EXPECT_EQ(Amount(12345 - 222), balances.get(Xem_Id));
	}

	TEST(TEST_CLASS, FullDebitRemovesMosaicFromCache) {
		// Arrange:
		AccountBalances balances;
		Amount amount = Amount(12345);
		balances.credit(Xem_Id, amount);

		// Act:
		balances.debit(Xem_Id, amount);
		auto xemHeld = balances.get(Xem_Id);

		// Assert:
		EXPECT_EQ(0u, balances.size());
		EXPECT_EQ(Amount(0), xemHeld);
	}

	TEST(TEST_CLASS, InterleavingDebitsYieldCorrectState) {
		// Arrange:
		AccountBalances balances;
		balances.credit(Xem_Id, Amount(12345));
		balances.credit(Test_Mosaic_Id, Amount(3456));

		// Act:
		balances.debit(Xem_Id, Amount(222));
		balances.debit(Test_Mosaic_Id, Amount(1111));
		balances.debit(Xem_Id, Amount(111));

		// Assert:
		EXPECT_EQ(2u, balances.size());
		EXPECT_EQ(Amount(12345 - 222 - 111), balances.get(Xem_Id));
		EXPECT_EQ(Amount(3456 - 1111), balances.get(Test_Mosaic_Id));
	}

	TEST(TEST_CLASS, DebitDoesNotAllowUnderflowOfNonZeroBalance) {
		// Arrange:
		AccountBalances balances;
		balances.credit(Xem_Id, Amount(12345));

		// Act + Assert:
		EXPECT_THROW(balances.debit(Xem_Id, Amount(12346)), catapult_runtime_error);

		// Assert:
		EXPECT_EQ(1u, balances.size());
		EXPECT_EQ(Amount(12345), balances.get(Xem_Id));
	}

	TEST(TEST_CLASS, DebitDoesNotAllowUnderflowOfZeroBalance) {
		// Arrange:
		AccountBalances balances;

		// Act + Assert:
		EXPECT_THROW(balances.debit(Xem_Id, Amount(222)), catapult_runtime_error);

		// Assert:
		EXPECT_EQ(0u, balances.size());
		EXPECT_EQ(Amount(0), balances.get(Xem_Id));
	}

	// endregion

	// region credit + debit

	TEST(TEST_CLASS, InterleavingDebitsAndCreditsYieldCorrectState) {
		// Arrange:
		AccountBalances balances;
		balances.credit(Xem_Id, Amount(12345));
		balances.credit(Test_Mosaic_Id, Amount(3456));

		// Act:
		balances.debit(Test_Mosaic_Id, Amount(1111));
		balances.credit(Xem_Id, Amount(1111));
		balances.credit(Test_Mosaic_Id2, Amount(0)); // no op
		balances.debit(Xem_Id, Amount(2345));
		balances.debit(Test_Mosaic_Id2, Amount(0)); // no op
		balances.credit(Test_Mosaic_Id, Amount(5432));

		// Assert:
		EXPECT_EQ(2u, balances.size());
		EXPECT_EQ(Amount(12345 + 1111 - 2345), balances.get(Xem_Id));
		EXPECT_EQ(Amount(3456 - 1111 + 5432), balances.get(Test_Mosaic_Id));
	}

	TEST(TEST_CLASS, ChainedInterleavingDebitsAndCreditsYieldCorrectState) {
		// Arrange:
		AccountBalances balances;
		balances
			.credit(Xem_Id, Amount(12345))
			.credit(Test_Mosaic_Id, Amount(3456));

		// Act:
		balances
			.debit(Test_Mosaic_Id, Amount(1111))
			.credit(Xem_Id, Amount(1111))
			.credit(Test_Mosaic_Id2, Amount(0)) // no op
			.debit(Xem_Id, Amount(2345))
			.debit(Test_Mosaic_Id2, Amount(0)) // no op
			.credit(Test_Mosaic_Id, Amount(5432));

		// Assert:
		EXPECT_EQ(2u, balances.size());
		EXPECT_EQ(Amount(12345 + 1111 - 2345), balances.get(Xem_Id));
		EXPECT_EQ(Amount(3456 - 1111 + 5432), balances.get(Test_Mosaic_Id));
	}

	// endregion

	// region iteration

	TEST(TEST_CLASS, CanIterateOverAllBalances) {
		// Arrange:
		AccountBalances balances;
		balances
			.credit(Xem_Id, Amount(12345))
			.credit(Test_Mosaic_Id2, Amount(0))
			.credit(Test_Mosaic_Id, Amount(3456));

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
		EXPECT_EQ(Amount(12345), iteratedBalances[Xem_Id]);
		EXPECT_EQ(Amount(3456), iteratedBalances[Test_Mosaic_Id]);
	}

	// endregion
}}
