#include "catapult/utils/BlockSpan.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS BlockSpanTests

	// region creation

	TEST(TEST_CLASS, CanCreateDefaultBlockSpan) {
		// Assert:
		EXPECT_EQ(0u, BlockSpan().hours());
	}

	TEST(TEST_CLASS, CanCreateBlockSpanFromDays) {
		// Assert:
		EXPECT_EQ(24u, BlockSpan::FromDays(1).hours());
		EXPECT_EQ(2 * 24u, BlockSpan::FromDays(2).hours());
		EXPECT_EQ(10 * 24u, BlockSpan::FromDays(10).hours());
		EXPECT_EQ(123 * 24u, BlockSpan::FromDays(123).hours());
	}

	TEST(TEST_CLASS, CanCreateBlockSpanFromHours) {
		// Assert:
		EXPECT_EQ(1u, BlockSpan::FromHours(1).hours());
		EXPECT_EQ(2u, BlockSpan::FromHours(2).hours());
		EXPECT_EQ(10u, BlockSpan::FromHours(10).hours());
		EXPECT_EQ(123u, BlockSpan::FromHours(123).hours());
	}

	// endregion

	// region accessor conversions

	TEST(TEST_CLASS, DaysAreTruncatedWhenConverted) {
		// Assert:
		constexpr uint64_t Base_Hours = 10 * 24u;
		EXPECT_EQ(9u, BlockSpan::FromHours(Base_Hours - 1).days());
		EXPECT_EQ(10u, BlockSpan::FromHours(Base_Hours).days());
		EXPECT_EQ(10u, BlockSpan::FromHours(Base_Hours + 1).days());
	}

	TEST(TEST_CLASS, BlocksAreDependentOnGenerationTime) {
		// Arrange:
		auto blockSpan = BlockSpan::FromHours(120);

		// Assert:
		EXPECT_EQ(BlockDuration(14'400u), blockSpan.blocks(TimeSpan::FromSeconds(30)));
		EXPECT_EQ(BlockDuration(7'200u), blockSpan.blocks(TimeSpan::FromMinutes(1)));
		EXPECT_EQ(BlockDuration(2'400u), blockSpan.blocks(TimeSpan::FromMinutes(3)));
	}

	TEST(TEST_CLASS, BlocksAreTruncatedWhenConverted) {
		// Arrange:
		auto blockSpan = BlockSpan::FromHours(119);

		// Assert:
		EXPECT_EQ(BlockDuration(32'953u), blockSpan.blocks(TimeSpan::FromSeconds(13)));
		EXPECT_EQ(BlockDuration(7'140u), blockSpan.blocks(TimeSpan::FromMinutes(1)));
		EXPECT_EQ(BlockDuration(649u), blockSpan.blocks(TimeSpan::FromMinutes(11)));
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
		// Assert:
		uint64_t max32 = std::numeric_limits<uint32_t>::max();
		Assert64BitBlockSpan(max32 + 1);
		Assert64BitBlockSpan(max32 + 1234);
		Assert64BitBlockSpan(max32 + 8692);
		Assert64BitBlockSpan(std::numeric_limits<uint64_t>::max() / (60 * 60'000));
	}

	TEST(TEST_CLASS, BlocksThrowsWhenOverflowIsDetectedCalculatingBlocksFromHours) {
		// Assert:
		uint64_t max64 = std::numeric_limits<uint64_t>::max();
		Assert64BitBlockSpanOverflow(max64 / (60 * 60'000) + 1);
		Assert64BitBlockSpanOverflow(max64);
	}

	// endregion

	// region equality operators

	namespace {
		std::unordered_set<std::string> GetEqualTags() {
			return { "96 h", "4 d", "96 h (2)" };
		}

		std::unordered_map<std::string, BlockSpan> GenerateEqualityInstanceMap() {
			return {
				{ "96 h", BlockSpan::FromHours(96) },
				{ "4 d", BlockSpan::FromDays(4) },
				{ "96 h (2)", BlockSpan::FromHours(96) },

				{ "95 h", BlockSpan::FromHours(95) },
				{ "97 h", BlockSpan::FromHours(97) },
				{ "96 d", BlockSpan::FromDays(96) }
			};
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects("96 h", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("96 h", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion

	// region comparison operators

	namespace {
		std::vector<BlockSpan> GenerateIncreasingValues() {
			return {
				BlockSpan::FromHours(95),
				BlockSpan::FromDays(4),
				BlockSpan::FromHours(97),
				BlockSpan::FromDays(96)
			};
		}
	}

	DEFINE_COMPARISON_TESTS(TEST_CLASS, GenerateIncreasingValues())

	// endregion

	// region to string

	namespace {
		void AssertStringRepresentation(const std::string& expected, uint64_t numDays, uint64_t numHours) {
			// Arrange:
			auto blockSpan = BlockSpan::FromHours((numDays * 24) + numHours);

			// Act:
			auto str = test::ToString(blockSpan);

			// Assert:
			EXPECT_EQ(expected, str) << numDays << "d " << numHours << "h";
		}
	}

	TEST(TEST_CLASS, CanOutputBlockSpan) {
		// Assert:
		// - zero
		AssertStringRepresentation("0d 0h", 0, 0);

		// - ones
		AssertStringRepresentation("1d 0h", 1, 0);
		AssertStringRepresentation("0d 1h", 0, 1);

		// - overflows
		AssertStringRepresentation("449d 0h", 449, 0);
		AssertStringRepresentation("2d 1h", 0, 49);

		// - all values
		AssertStringRepresentation("1d 1h", 1, 1);
		AssertStringRepresentation("59d 23h", 59, 23);
		AssertStringRepresentation("12d 7h", 12, 7);
	}

	// endregion
}}
