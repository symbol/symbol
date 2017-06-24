#include "catapult/state/BlockDifficultyInfo.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

	namespace {
		auto data1 = BlockDifficultyInfo(Height(123), Timestamp(234), Difficulty(345));
		auto data2 = BlockDifficultyInfo(Height(123), Timestamp(235), Difficulty(345));
		auto data3 = BlockDifficultyInfo(Height(123), Timestamp(234), Difficulty(346));
		auto data4 = BlockDifficultyInfo(Height(124), Timestamp(234), Difficulty(345));
	}

	TEST(BlockDifficultyInfoTests, CanCreateDefaultBlockDifficultyInfo) {
		// Act:
		auto info = BlockDifficultyInfo();

		// Assert:
		EXPECT_EQ(Height(0), info.BlockHeight);
		EXPECT_EQ(Timestamp(0), info.BlockTimestamp);
		EXPECT_EQ(Difficulty(0), info.BlockDifficulty);
	}

	TEST(BlockDifficultyInfoTests, CanCreateBlockDifficultyInfoFromParameters) {
		// Act:
		auto info = BlockDifficultyInfo(Height(123), Timestamp(234), Difficulty(345));

		// Assert:
		EXPECT_EQ(Height(123), info.BlockHeight);
		EXPECT_EQ(Timestamp(234), info.BlockTimestamp);
		EXPECT_EQ(Difficulty(345), info.BlockDifficulty);
	}

	TEST(BlockDifficultyInfoTests, CanCreateBlockDifficultyInfoFromHeightOnly) {
		// Act:
		auto info = BlockDifficultyInfo(Height(123));

		// Assert:
		EXPECT_EQ(Height(123), info.BlockHeight);
		EXPECT_EQ(Timestamp(0), info.BlockTimestamp);
		EXPECT_EQ(Difficulty(0), info.BlockDifficulty);
	}

	TEST(BlockDifficultyInfoTests, OperatorLessThanReturnsTrueForSmallerValuesAndFalseOtherwise) {
		// Assert:
		EXPECT_FALSE(data1 < data2);
		EXPECT_FALSE(data2 < data1);
		EXPECT_FALSE(data1 < data3);
		EXPECT_FALSE(data3 < data1);
		EXPECT_TRUE(data1 < data4);
		EXPECT_FALSE(data4 < data1);
	}

	TEST(BlockDifficultyInfoTests, OperatorEqualReturnsTrueOnlyForEqualValues) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects({ data1, data2, data3 }, { data4 } );
	}

	TEST(BlockDifficultyInfoTests, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects({ data1, data2, data3 }, { data4 });
	}
}}
