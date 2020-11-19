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

#include "harvesting/src/TransactionFeeMaximizer.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS TransactionFeeMaximizerTests

	// region test utils

	namespace {
		constexpr auto CreateTransactionInfos = test::CreateTransactionInfosFromSizeMultiplierPairs;

		void ApplyAll(TransactionFeeMaximizer& maximizer, const std::vector<model::TransactionInfo>& transactionInfos) {
			for (const auto& transactionInfo : transactionInfos)
				maximizer.apply(transactionInfo);
		}

		void AssertFeePolicy(const FeePolicy& policy, uint32_t numTransactions, BlockFeeMultiplier feeMultiplier, Amount baseFee) {
			// Assert:
			EXPECT_EQ(numTransactions, policy.NumTransactions);
			EXPECT_EQ(feeMultiplier, policy.FeeMultiplier);
			EXPECT_EQ(baseFee, policy.BaseFee);
		}
	}

	// endregion

	// region zero or one transactions

	TEST(TEST_CLASS, CanFindBestMultiplier_ZeroTransactions) {
		// Act:
		TransactionFeeMaximizer maximizer;

		// Assert: best multiplier should be zero
		AssertFeePolicy(maximizer.best(), 0, BlockFeeMultiplier(0), Amount(0));
	}

	TEST(TEST_CLASS, CanFindBestMultiplier_SingleTransaction_MaxFeeSizeMultiple) {
		// Arrange:
		TransactionFeeMaximizer maximizer;

		// Act:
		auto transactionInfos = CreateTransactionInfos({ { 200, 500 } });
		ApplyAll(maximizer, transactionInfos);

		// Assert: best multipler is equal to max transaction multiplier (exact)
		AssertFeePolicy(maximizer.best(), 1, BlockFeeMultiplier(50), Amount(200));
	}

	TEST(TEST_CLASS, CanFindBestMultiplier_SingleTransaction_MaxFeeNotSizeMultiple) {
		// Arrange:
		TransactionFeeMaximizer maximizer;

		// Act:
		auto transactionInfos = CreateTransactionInfos({ { 333, 567 } });
		ApplyAll(maximizer, transactionInfos);

		// Assert: best multipler is equal to max transaction multiplier (truncated)
		AssertFeePolicy(maximizer.best(), 1, BlockFeeMultiplier(56), Amount(333));
	}

	// endregion

	// region multiple transactions

	TEST(TEST_CLASS, CanFindBestMultiplier_MultipleTransactions_AllSameMaxMultiple) {
		// Arrange:
		TransactionFeeMaximizer maximizer;

		// Act: all max multiples are 50.x
		auto transactionInfos = CreateTransactionInfos({ { 200, 503 }, { 500, 502 }, { 444, 501 }, { 234, 500 } });
		ApplyAll(maximizer, transactionInfos);

		// Assert: all transactions are included
		AssertFeePolicy(maximizer.best(), 4, BlockFeeMultiplier(50), Amount(1378));
	}

	TEST(TEST_CLASS, CanFindBestMultiplier_MultipleTransactions_OneDominatesWithLargeMultiple) {
		// Arrange:
		TransactionFeeMaximizer maximizer;

		// Act: first transaction has multiple 10x larger than rest and dominates even with a smaller size
		auto transactionInfos = CreateTransactionInfos({ { 200, 5003 }, { 500, 502 }, { 444, 501 }, { 234, 500 } });
		ApplyAll(maximizer, transactionInfos);

		// Assert: only first transaction is included
		AssertFeePolicy(maximizer.best(), 1, BlockFeeMultiplier(500), Amount(200));
	}

	TEST(TEST_CLASS, CanFindBestMultiplier_MultipleTransactions_OneDominatesWithLargeSize) {
		// Arrange:
		TransactionFeeMaximizer maximizer;

		// Act: last transaction has payload much larger than rest and dominates even with a smaller multiplier
		auto transactionInfos = CreateTransactionInfos({ { 200, 503 }, { 500, 502 }, { 444, 501 }, { 234, 500 }, { 6000, 100 } });
		ApplyAll(maximizer, transactionInfos);

		// Assert: all transactions are included
		AssertFeePolicy(maximizer.best(), 5, BlockFeeMultiplier(10), Amount(7378));
	}

	TEST(TEST_CLASS, CanFindBestMultiplier_MultipleTransactions_ScoreImprovesThenWorsens) {
		// Arrange:
		TransactionFeeMaximizer maximizer;

		// Act: scores (1, 2) > (1) > (1, 2, 3) > (1, 2, 3, 4)
		auto transactionInfos = CreateTransactionInfos({ { 200, 500 }, { 500, 400 }, { 250, 100 }, { 400, 50 } });
		ApplyAll(maximizer, transactionInfos);

		// Assert: first two transactions are included
		AssertFeePolicy(maximizer.best(), 2, BlockFeeMultiplier(40), Amount(700));
	}

	TEST(TEST_CLASS, CanFindBestMultiplier_MultipleTransactions_ScoreWorsensThenImproves) {
		// Arrange:
		TransactionFeeMaximizer maximizer;

		// Act: scores (1, 2, 3) > (1) > (1, 2) > (1, 2, 3, 4)
		auto transactionInfos = CreateTransactionInfos({ { 200, 500 }, { 200, 200 }, { 600, 150 }, { 400, 20 } });
		ApplyAll(maximizer, transactionInfos);

		// Assert: first three transactions are included
		AssertFeePolicy(maximizer.best(), 3, BlockFeeMultiplier(15), Amount(1000));
	}

	TEST(TEST_CLASS, CanFindBestMultiplier_MultipleTransactions_PrefersMoreTransactions) {
		// Arrange:
		TransactionFeeMaximizer maximizer;

		// Act: scores (1) == (1, 2) == (1, 2, 3) > (1, 2, 3, 4)
		auto transactionInfos = CreateTransactionInfos({ { 200, 400 }, { 200, 200 }, { 400, 100 }, { 400, 20 } });
		ApplyAll(maximizer, transactionInfos);

		// Assert: first three transactions are included
		AssertFeePolicy(maximizer.best(), 3, BlockFeeMultiplier(10), Amount(800));
	}

	// endregion

	// region multiple transactions (multiplier strict ordering)

	TEST(TEST_CLASS, CanFindBestMultiplier_MultipleTransactions_WholeMultipliersCannotIncrease) {
		// Arrange:
		TransactionFeeMaximizer maximizer;

		// Act:
		auto transactionInfos = CreateTransactionInfos({ { 200, 500 }, { 200, 510 }, { 600, 480 } });
		maximizer.apply(transactionInfos[0]);

		// Assert: cannot increase multipliers (50 to 51)
		EXPECT_THROW(maximizer.apply(transactionInfos[1]), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanFindBestMultiplier_MultipleTransactions_FractionalMultipliersCanIncrease) {
		// Arrange:
		TransactionFeeMaximizer maximizer;

		// Act:
		auto transactionInfos = CreateTransactionInfos({ { 201, 500 }, { 202, 509 }, { 203, 507 } });
		ApplyAll(maximizer, transactionInfos);

		// Assert: only whole multipliers matter
		AssertFeePolicy(maximizer.best(), 3, BlockFeeMultiplier(50), Amount(606));
	}

	TEST(TEST_CLASS, CanFindBestMultiplier_MultipleTransactions_MultipliersCanBeRepeated) {
		// Arrange:
		TransactionFeeMaximizer maximizer;

		// Act:
		auto transactionInfos = CreateTransactionInfos({ { 201, 500 }, { 202, 500 }, { 203, 507 }, { 204, 507 } });
		ApplyAll(maximizer, transactionInfos);

		// Assert: repeats are allowed
		AssertFeePolicy(maximizer.best(), 4, BlockFeeMultiplier(50), Amount(810));
	}

	// endregion
}}
