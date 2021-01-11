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

#include "catapult/chain/TransactionUpdateResultUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS TransactionUpdateResultUtilsTests

	// region UpdateResult

	namespace {
		struct UpdateResult {
			enum class UpdateType {
				New_Alpha,
				New_Beta,
				Invalid,
				Neutral
			};

			UpdateType Type;
		};

		constexpr auto New_Alpha = UpdateResult::UpdateType::New_Alpha;
		constexpr auto New_Beta = UpdateResult::UpdateType::New_Beta;
		constexpr auto Neutral = UpdateResult::UpdateType::Neutral;
		constexpr auto Invalid = UpdateResult::UpdateType::Invalid;
	}

	// endregion

	// region AggregateUpdateResults

	TEST(TEST_CLASS, AggregateUpdateResults_AggregatesByCount) {
		// Arrange:
		auto updateResults = std::vector<UpdateResult>{
			{ New_Alpha },
			{ Invalid },
			{ Neutral },
			{ Neutral },
			{ New_Beta },
			{ Invalid },
			{ New_Beta },
			{ Neutral },
			{ New_Alpha }
		};

		// Act:
		auto aggregateResult = AggregateUpdateResults(updateResults);

		// Assert:
		EXPECT_EQ(BatchUpdateResult(4, 3, 2), aggregateResult);
	}

	// endregion

	// region SelectValid

	TEST(TEST_CLASS, SelectValid_FailsWhenFewerTransactionInfosThanUpdateResults) {
		// Arrange:
		auto transactionInfos = test::CreateTransactionInfos(2);
		auto updateResults = std::vector<UpdateResult>{
			{ New_Alpha },
			{ New_Alpha },
			{ New_Alpha }
		};

		// Act + Assert:
		EXPECT_THROW(SelectValid(std::move(transactionInfos), updateResults), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, SelectValid_FailsWhenMoreTransactionInfosThanUpdateResults) {
		// Arrange:
		auto transactionInfos = test::CreateTransactionInfos(4);
		auto updateResults = std::vector<UpdateResult>{
			{ New_Alpha },
			{ New_Alpha },
			{ New_Alpha }
		};

		// Act + Assert:
		EXPECT_THROW(SelectValid(std::move(transactionInfos), updateResults), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, SelectValid_FiltersOutInvalidTransactions) {
		// Arrange:
		auto transactionInfos = test::CreateTransactionInfos(9);
		auto updateResults = std::vector<UpdateResult>{
			{ New_Alpha },
			{ Invalid },
			{ Neutral },
			{ Neutral },
			{ New_Beta },
			{ Invalid },
			{ New_Beta },
			{ Neutral },
			{ New_Alpha }
		};

		// Act:
		auto filteredTransactionInfos = SelectValid(test::CopyTransactionInfos(transactionInfos), updateResults);

		// Assert:
		ASSERT_EQ(4u, filteredTransactionInfos.size());
		test::AssertEqual(transactionInfos[0], filteredTransactionInfos[0], "0");
		test::AssertEqual(transactionInfos[4], filteredTransactionInfos[1], "1");
		test::AssertEqual(transactionInfos[6], filteredTransactionInfos[2], "2");
		test::AssertEqual(transactionInfos[8], filteredTransactionInfos[3], "3");
	}

	// endregion
}}
