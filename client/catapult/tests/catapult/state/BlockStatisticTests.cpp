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

#include "catapult/state/BlockStatistic.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS BlockStatisticTests

	namespace {
		constexpr Difficulty CreateDifficulty(uint64_t delta) {
			// create a difficulty `delta` greater than default difficulty value
			return Difficulty() + Difficulty::Unclamped(delta);
		}
	}

	// region ctor

	TEST(TEST_CLASS, CanCreateDefaultBlockStatistic) {
		// Act:
		auto statistic = BlockStatistic();

		// Assert:
		EXPECT_EQ(Height(0), statistic.Height);
		EXPECT_EQ(Timestamp(0), statistic.Timestamp);
		EXPECT_EQ(Difficulty(0), statistic.Difficulty);
		EXPECT_EQ(BlockFeeMultiplier(0), statistic.FeeMultiplier);
	}

	TEST(TEST_CLASS, CanCreateBlockStatisticFromHeight) {
		// Act:
		auto statistic = BlockStatistic(Height(123));

		// Assert:
		EXPECT_EQ(Height(123), statistic.Height);
		EXPECT_EQ(Timestamp(0), statistic.Timestamp);
		EXPECT_EQ(Difficulty(0), statistic.Difficulty);
		EXPECT_EQ(BlockFeeMultiplier(0), statistic.FeeMultiplier);
	}

	TEST(TEST_CLASS, CanCreateBlockStatisticFromBlock) {
		// Arrange:
		model::Block block;
		block.Height = Height(123);
		block.Timestamp = Timestamp(234);
		block.Difficulty = CreateDifficulty(345);
		block.FeeMultiplier = BlockFeeMultiplier(456);

		// Act:
		auto statistic = BlockStatistic(block);

		// Assert:
		EXPECT_EQ(Height(123), statistic.Height);
		EXPECT_EQ(Timestamp(234), statistic.Timestamp);
		EXPECT_EQ(CreateDifficulty(345), statistic.Difficulty);
		EXPECT_EQ(BlockFeeMultiplier(456), statistic.FeeMultiplier);
	}

	TEST(TEST_CLASS, CanCreateBlockStatisticFromValues) {
		// Act:
		auto statistic = BlockStatistic(Height(123), Timestamp(234), CreateDifficulty(345), BlockFeeMultiplier(456));

		// Assert:
		EXPECT_EQ(Height(123), statistic.Height);
		EXPECT_EQ(Timestamp(234), statistic.Timestamp);
		EXPECT_EQ(CreateDifficulty(345), statistic.Difficulty);
		EXPECT_EQ(BlockFeeMultiplier(456), statistic.FeeMultiplier);
	}

	// endregion

	// region comparison operators

	namespace {
		constexpr auto Statistic1 = BlockStatistic(Height(123), Timestamp(234), CreateDifficulty(345), BlockFeeMultiplier(456));
		constexpr auto Statistic2 = BlockStatistic(Height(123), Timestamp(234), CreateDifficulty(345), BlockFeeMultiplier(457));
		constexpr auto Statistic3 = BlockStatistic(Height(123), Timestamp(234), CreateDifficulty(344), BlockFeeMultiplier(456));
		constexpr auto Statistic4 = BlockStatistic(Height(123), Timestamp(233), CreateDifficulty(345), BlockFeeMultiplier(456));
		constexpr auto Statistic5 = BlockStatistic(Height(124), Timestamp(234), CreateDifficulty(345), BlockFeeMultiplier(456));
	}

	TEST(TEST_CLASS, OperatorLessThanReturnsTrueForSmallerValuesAndFalseOtherwise) {
		test::AssertLessThanOperatorForEqualValues(Statistic1, Statistic2);
		test::AssertLessThanOperatorForEqualValues(Statistic1, Statistic3);
		test::AssertLessThanOperatorForEqualValues(Statistic1, Statistic4);
		test::AssertLessThanOperatorForEqualValues(Statistic2, Statistic3);
		test::AssertLessThanOperatorForEqualValues(Statistic2, Statistic4);
		test::AssertLessThanOperatorForEqualValues(Statistic3, Statistic4);

		EXPECT_TRUE(Statistic1 < Statistic5);
		EXPECT_TRUE(Statistic2 < Statistic5);
		EXPECT_TRUE(Statistic3 < Statistic5);
		EXPECT_TRUE(Statistic4 < Statistic5);
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects({ Statistic1, Statistic2, Statistic3, Statistic4 }, { Statistic5 } );
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects({ Statistic1, Statistic2, Statistic3, Statistic4 }, { Statistic5 });
	}

	// endregion

	// region to string

	TEST(TEST_CLASS, CanOutputBlockStatistic) {
		// Arrange:
		auto statistic = BlockStatistic(Height(123), Timestamp(234), CreateDifficulty(345), BlockFeeMultiplier(456));

		// Act:
		auto str = test::ToString(statistic);

		// Assert:
		EXPECT_EQ("height = 123, timestamp = 234, difficulty = 100000000000345, feeMultiplier = 456", str);
	}

	// endregion
}}
