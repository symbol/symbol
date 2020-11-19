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

#include "catapult/utils/BlockSpan.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS BlockSpanTests

	// region creation

	TEST(TEST_CLASS, CanCreateDefaultBlockSpan) {
		EXPECT_EQ(0u, BlockSpan().minutes());
	}

	TEST(TEST_CLASS, CanCreateBlockSpanFromMinutes) {
		EXPECT_EQ(1u, BlockSpan::FromMinutes(1).minutes());
		EXPECT_EQ(2u, BlockSpan::FromMinutes(2).minutes());
		EXPECT_EQ(10u, BlockSpan::FromMinutes(10).minutes());
		EXPECT_EQ(123u, BlockSpan::FromMinutes(123).minutes());
	}

	TEST(TEST_CLASS, CanCreateBlockSpanFromHours) {
		EXPECT_EQ(60u, BlockSpan::FromHours(1).minutes());
		EXPECT_EQ(2 * 60u, BlockSpan::FromHours(2).minutes());
		EXPECT_EQ(10 * 60u, BlockSpan::FromHours(10).minutes());
		EXPECT_EQ(123 * 60u, BlockSpan::FromHours(123).minutes());
	}

	TEST(TEST_CLASS, CanCreateBlockSpanFromDays) {
		EXPECT_EQ(24 * 60u, BlockSpan::FromDays(1).minutes());
		EXPECT_EQ(2 * 24 * 60u, BlockSpan::FromDays(2).minutes());
		EXPECT_EQ(10 * 24 * 60u, BlockSpan::FromDays(10).minutes());
		EXPECT_EQ(123 * 24 * 60u, BlockSpan::FromDays(123).minutes());
	}

	// endregion

	// region accessor conversions

	TEST(TEST_CLASS, HoursAreTruncatedWhenConverted) {
		constexpr uint64_t Base_Minutes = 10 * 60u;
		EXPECT_EQ(9u, BlockSpan::FromMinutes(Base_Minutes - 1).hours());
		EXPECT_EQ(10u, BlockSpan::FromMinutes(Base_Minutes).hours());
		EXPECT_EQ(10u, BlockSpan::FromMinutes(Base_Minutes + 1).hours());
	}

	TEST(TEST_CLASS, DaysAreTruncatedWhenConverted) {
		constexpr uint64_t Base_Hours = 10 * 24u;
		EXPECT_EQ(9u, BlockSpan::FromHours(Base_Hours - 1).days());
		EXPECT_EQ(10u, BlockSpan::FromHours(Base_Hours).days());
		EXPECT_EQ(10u, BlockSpan::FromHours(Base_Hours + 1).days());
	}

	TEST(TEST_CLASS, BlocksAreDependentOnGenerationTime) {
		// Arrange:
		auto blockSpan = BlockSpan::FromHours(120);

		// Assert:
		EXPECT_EQ(BlockDuration(14'400), blockSpan.blocks(TimeSpan::FromSeconds(30)));
		EXPECT_EQ(BlockDuration(7'200), blockSpan.blocks(TimeSpan::FromMinutes(1)));
		EXPECT_EQ(BlockDuration(2'400), blockSpan.blocks(TimeSpan::FromMinutes(3)));
	}

	TEST(TEST_CLASS, BlocksAreTruncatedWhenConverted) {
		// Arrange:
		auto blockSpan = BlockSpan::FromHours(119);

		// Assert:
		EXPECT_EQ(BlockDuration(32'953), blockSpan.blocks(TimeSpan::FromSeconds(13)));
		EXPECT_EQ(BlockDuration(7'140), blockSpan.blocks(TimeSpan::FromMinutes(1)));
		EXPECT_EQ(BlockDuration(649), blockSpan.blocks(TimeSpan::FromMinutes(11)));
	}

	namespace {
		void Assert64BitBlockSpan(uint64_t value) {
			// Arrange:
			auto blockSpan = BlockSpan::FromHours(value);

			// Act + Assert: the value is accessible via blocks
			EXPECT_EQ(BlockDuration(value), blockSpan.blocks(TimeSpan::FromHours(1)));
		}

		void Assert64BitBlockSpanOverflow(uint64_t value) {
			// Arrange:
			auto blockSpan = BlockSpan::FromHours(value);

			// Act + Assert: the value is not accessible via blocks
			EXPECT_THROW(blockSpan.blocks(TimeSpan::FromHours(1)), catapult_runtime_error);
		}
	}

	TEST(TEST_CLASS, BlocksSupportsValuesAbove32Bits) {
		uint64_t max32 = std::numeric_limits<uint32_t>::max();
		Assert64BitBlockSpan(max32 + 1);
		Assert64BitBlockSpan(max32 + 1234);
		Assert64BitBlockSpan(max32 + 8692);
		Assert64BitBlockSpan(std::numeric_limits<uint64_t>::max() / (60 * 60'000));
	}

	TEST(TEST_CLASS, BlocksThrowsWhenOverflowIsDetectedCalculatingBlocksFromHours) {
		uint64_t max64 = std::numeric_limits<uint64_t>::max();
		Assert64BitBlockSpanOverflow(max64 / (60 * 60'000) + 1);
		Assert64BitBlockSpanOverflow(max64);
	}

	// endregion

	// region equality operators

	namespace {
		std::unordered_set<std::string> GetEqualTags() {
			return { "96 h", "4 d", "96 h (2)", "5760 m" };
		}

		std::unordered_map<std::string, BlockSpan> GenerateEqualityInstanceMap() {
			return {
				{ "96 h", BlockSpan::FromHours(96) },
				{ "4 d", BlockSpan::FromDays(4) },
				{ "96 h (2)", BlockSpan::FromHours(96) },
				{ "5760 m", BlockSpan::FromMinutes(5760) },

				{ "95 h", BlockSpan::FromHours(95) },
				{ "97 h", BlockSpan::FromHours(97) },
				{ "96 d", BlockSpan::FromDays(96) },
				{ "5761 m", BlockSpan::FromMinutes(5761) }
			};
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects("96 h", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("96 h", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion

	// region comparison operators

	namespace {
		std::vector<BlockSpan> GenerateIncreasingValues() {
			return {
				BlockSpan::FromHours(95),
				BlockSpan::FromDays(4),
				BlockSpan::FromMinutes(96 * 60 + 30),
				BlockSpan::FromHours(97),
				BlockSpan::FromDays(96)
			};
		}
	}

	DEFINE_COMPARISON_TESTS(TEST_CLASS, GenerateIncreasingValues())

	// endregion

	// region to string

	namespace {
		void AssertStringRepresentation(const std::string& expected, uint64_t numDays, uint64_t numHours, uint64_t numMinutes) {
			// Arrange:
			auto blockSpan = BlockSpan::FromMinutes(((numDays * 24) + numHours) * 60 + numMinutes);

			// Act:
			auto str = test::ToString(blockSpan);

			// Assert:
			EXPECT_EQ(expected, str) << numDays << "d " << numHours << "h " << numMinutes << "m";
		}
	}

	TEST(TEST_CLASS, CanOutputBlockSpan) {
		// Assert: zero
		AssertStringRepresentation("0d 0h 0m", 0, 0, 0);

		// - ones
		AssertStringRepresentation("1d 0h 0m", 1, 0, 0);
		AssertStringRepresentation("0d 1h 0m", 0, 1, 0);
		AssertStringRepresentation("0d 0h 1m", 0, 0, 1);

		// - overflows
		AssertStringRepresentation("449d 0h 0m", 449, 0, 0);
		AssertStringRepresentation("2d 1h 0m", 0, 49, 0);
		AssertStringRepresentation("3d 2h 21m", 0, 0, 4461);

		// - all values
		AssertStringRepresentation("1d 1h 1m", 1, 1, 1);
		AssertStringRepresentation("99d 23h 59m", 99, 23, 59);
		AssertStringRepresentation("12d 7h 9m", 12, 7, 9);
	}

	// endregion
}}
