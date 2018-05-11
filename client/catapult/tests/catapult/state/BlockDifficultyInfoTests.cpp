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

#include "catapult/state/BlockDifficultyInfo.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS BlockDifficultyInfoTests

	namespace {
		auto data1 = BlockDifficultyInfo(Height(123), Timestamp(234), Difficulty(345));
		auto data2 = BlockDifficultyInfo(Height(123), Timestamp(235), Difficulty(345));
		auto data3 = BlockDifficultyInfo(Height(123), Timestamp(234), Difficulty(346));
		auto data4 = BlockDifficultyInfo(Height(124), Timestamp(234), Difficulty(345));
	}

	TEST(TEST_CLASS, CanCreateDefaultBlockDifficultyInfo) {
		// Act:
		auto info = BlockDifficultyInfo();

		// Assert:
		EXPECT_EQ(Height(0), info.BlockHeight);
		EXPECT_EQ(Timestamp(0), info.BlockTimestamp);
		EXPECT_EQ(Difficulty(0), info.BlockDifficulty);
	}

	TEST(TEST_CLASS, CanCreateBlockDifficultyInfoFromParameters) {
		// Act:
		auto info = BlockDifficultyInfo(Height(123), Timestamp(234), Difficulty(345));

		// Assert:
		EXPECT_EQ(Height(123), info.BlockHeight);
		EXPECT_EQ(Timestamp(234), info.BlockTimestamp);
		EXPECT_EQ(Difficulty(345), info.BlockDifficulty);
	}

	TEST(TEST_CLASS, CanCreateBlockDifficultyInfoFromHeightOnly) {
		// Act:
		auto info = BlockDifficultyInfo(Height(123));

		// Assert:
		EXPECT_EQ(Height(123), info.BlockHeight);
		EXPECT_EQ(Timestamp(0), info.BlockTimestamp);
		EXPECT_EQ(Difficulty(0), info.BlockDifficulty);
	}

	TEST(TEST_CLASS, OperatorLessThanReturnsTrueForSmallerValuesAndFalseOtherwise) {
		// Assert:
		EXPECT_FALSE(data1 < data2);
		EXPECT_FALSE(data2 < data1);
		EXPECT_FALSE(data1 < data3);
		EXPECT_FALSE(data3 < data1);
		EXPECT_TRUE(data1 < data4);
		EXPECT_FALSE(data4 < data1);
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects({ data1, data2, data3 }, { data4 } );
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects({ data1, data2, data3 }, { data4 });
	}
}}
