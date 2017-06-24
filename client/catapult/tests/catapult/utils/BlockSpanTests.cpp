#include "catapult/utils/BlockSpan.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

	// region creation

	TEST(BlockSpanTests, CanCreateDefaultBlockSpan) {
		// Assert:
		EXPECT_EQ(0u, BlockSpan().hours());
	}

	TEST(BlockSpanTests, CanCreateBlockSpanFromDays) {
		// Assert:
		EXPECT_EQ(24u, BlockSpan::FromDays(1).hours());
		EXPECT_EQ(2 * 24u, BlockSpan::FromDays(2).hours());
		EXPECT_EQ(10 * 24u, BlockSpan::FromDays(10).hours());
		EXPECT_EQ(123 * 24u, BlockSpan::FromDays(123).hours());
	}

	TEST(BlockSpanTests, CanCreateBlockSpanFromHours) {
		// Assert:
		EXPECT_EQ(1u, BlockSpan::FromHours(1).hours());
		EXPECT_EQ(2u, BlockSpan::FromHours(2).hours());
		EXPECT_EQ(10u, BlockSpan::FromHours(10).hours());
		EXPECT_EQ(123u, BlockSpan::FromHours(123).hours());
	}

	// endregion

	// region accessor conversions

	TEST(BlockSpanTests, DaysAreTruncatedWhenConverted) {
		// Assert:
		constexpr uint64_t Base_Hours = 10 * 24u;
		EXPECT_EQ(9u, BlockSpan::FromHours(Base_Hours - 1).days());
		EXPECT_EQ(10u, BlockSpan::FromHours(Base_Hours).days());
		EXPECT_EQ(10u, BlockSpan::FromHours(Base_Hours + 1).days());
	}

	TEST(BlockSpanTests, BlocksAreDependentOnGenerationTime) {
		// Arrange:
		auto blockSpan = BlockSpan::FromHours(120);

		// Assert:
		EXPECT_EQ(14'400u, blockSpan.blocks(TimeSpan::FromSeconds(30)));
		EXPECT_EQ(7'200u, blockSpan.blocks(TimeSpan::FromMinutes(1)));
		EXPECT_EQ(2'400u, blockSpan.blocks(TimeSpan::FromMinutes(3)));
	}

	TEST(BlockSpanTests, BlocksAreTruncatedWhenConverted) {
		// Arrange:
		auto blockSpan = BlockSpan::FromHours(119);

		// Assert:
		EXPECT_EQ(32'953u, blockSpan.blocks(TimeSpan::FromSeconds(13)));
		EXPECT_EQ(7'140u, blockSpan.blocks(TimeSpan::FromMinutes(1)));
		EXPECT_EQ(649u, blockSpan.blocks(TimeSpan::FromMinutes(11)));
	}

	namespace {
		void Assert32BitBlockSpan(uint32_t value) {
			// Act:
			auto blockSpan = BlockSpan::FromHours(value);

			// Assert: the value is accessible via blocks32 and blocks
			EXPECT_EQ(value, blockSpan.blocks32(TimeSpan::FromHours(1)));
			EXPECT_EQ(value, blockSpan.blocks(TimeSpan::FromHours(1)));
		}

		void Assert64BitBlockSpan(uint64_t value) {
			// Act:
			auto blockSpan = BlockSpan::FromHours(value);

			// Assert: the value is accessible via blocks but not blocks32
			EXPECT_THROW(blockSpan.blocks32(TimeSpan::FromHours(1)), catapult_runtime_error);
			EXPECT_EQ(value, blockSpan.blocks(TimeSpan::FromHours(1)));
		}

		void Assert64BitBlockSpanOverflow(uint64_t value) {
			// Act:
			auto blockSpan = BlockSpan::FromHours(value);

			// Assert: the value is accessible neither via blocks nor blocks32
			EXPECT_THROW(blockSpan.blocks32(TimeSpan::FromHours(1)), catapult_runtime_error);
			EXPECT_THROW(blockSpan.blocks(TimeSpan::FromHours(1)), catapult_runtime_error);
		}
	}

	TEST(BlockSpanTests, Blocks32ReturnsBlocksWhenBlocks64FitsInto32Bit) {
		// Assert:
		using NumericLimits = std::numeric_limits<uint32_t>;
		Assert32BitBlockSpan(NumericLimits::min()); // min
		Assert32BitBlockSpan(1); // other values
		Assert32BitBlockSpan(1234);
		Assert32BitBlockSpan(8692);
		Assert32BitBlockSpan(NumericLimits::max()); // max
	}

	TEST(BlockSpanTests, Blocks32ThrowsWhenBlocks64DoesNotFitInto32Bit) {
		// Assert:
		uint64_t max32 = std::numeric_limits<uint32_t>::max();
		Assert64BitBlockSpan(max32 + 1);
		Assert64BitBlockSpan(max32 + 1234);
		Assert64BitBlockSpan(max32 + 8692);
		Assert64BitBlockSpan(std::numeric_limits<uint64_t>::max() / (60 * 60'000));
	}

	TEST(BlockSpanTests, BlocksThrowsWhenOverflowIsDetectedCalculatingBlocksFromHours) {
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

	TEST(BlockSpanTests, OperatorEqualReturnsTrueOnlyForEqualValues) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects("96 h", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(BlockSpanTests, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
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

	DEFINE_COMPARISON_TESTS(BlockSpanTests, GenerateIncreasingValues())

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

	TEST(BlockSpanTests, CanOutputBlockSpan) {
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
