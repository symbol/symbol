#include "catapult/utils/BitwiseEnum.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS BitwiseEnumTests

	namespace {
		enum class TestEnum {
			None = 0x00,
			Alpha = 0x01,
			Beta = 0x02,
			Gamma = 0x04,
			Delta = 0x20,
			All = 0xFF
		};

		MAKE_BITWISE_ENUM(TestEnum)
	}

	// region or operator

	TEST(TEST_CLASS, CanOrBitwiseFlags) {
		// Act + Assert:
		EXPECT_EQ(0x05, utils::to_underlying_type(TestEnum::Alpha | TestEnum::Gamma));
	}

	TEST(TEST_CLASS, OrIsIdempotent) {
		// Act + Assert:
		EXPECT_EQ(0x03, utils::to_underlying_type(TestEnum::Alpha | TestEnum::Beta | TestEnum::Beta));
	}

	TEST(TEST_CLASS, OrEqualsYieldsCorrectResult) {
		// Arrange:
		auto combinedFlags = TestEnum::None;

		// Act: alpha and beta flags duplicated on purpose
		for (auto flag : { TestEnum::Alpha, TestEnum::Beta, TestEnum::Gamma, TestEnum::Delta, TestEnum::Alpha, TestEnum::Beta })
			combinedFlags |= flag;

		// Assert:
		EXPECT_EQ(0x27, utils::to_underlying_type(combinedFlags));
		EXPECT_EQ(TestEnum::Alpha | TestEnum::Beta | TestEnum::Gamma | TestEnum::Delta, combinedFlags);
	}

	TEST(TEST_CLASS, OrEqualsSupportsChaining) {
		// Arrange:
		auto combinedFlags = TestEnum::None;

		// Act:
		(combinedFlags |= TestEnum::Delta) |= TestEnum::Gamma;

		// Assert:
		EXPECT_EQ(TestEnum::Delta | TestEnum::Gamma, combinedFlags);
	}

	// endregion

	// region has flag

	TEST(TEST_CLASS, HasFlagReturnsTrueWhenAllFlagsAreSet) {
		// Act + Assert:
		for (auto flag : { TestEnum::Alpha, TestEnum::Beta, TestEnum::Gamma, TestEnum::Delta, TestEnum::All })
			EXPECT_TRUE(HasFlag(flag, TestEnum::All)) << "flag " << utils::to_underlying_type(flag);
	}

	TEST(TEST_CLASS, HasFlagReturnsFalseWhenAllFlagsAreUnset) {
		// Act + Assert:
		for (auto flag : { TestEnum::Alpha, TestEnum::Beta, TestEnum::Gamma, TestEnum::Delta, TestEnum::All })
			EXPECT_FALSE(HasFlag(flag, TestEnum::None)) << "flag " << utils::to_underlying_type(flag);
	}

	TEST(TEST_CLASS, HasFlagAlwaysReturnsTrueWhenTestedFlagIsZero) {
		// Act + Assert:
		for (auto flags : { TestEnum::None, TestEnum::Alpha, TestEnum::Beta, TestEnum::Gamma, TestEnum::Delta, TestEnum::All })
			EXPECT_TRUE(HasFlag(TestEnum::None, flags)) << "flags " << utils::to_underlying_type(flags);
	}

	TEST(TEST_CLASS, HasFlagReturnsTrueIfAndOnlyIfFlagIsSet) {
		// Arrange:
		auto flags = TestEnum::Alpha | TestEnum::Gamma;

		// Act + Assert:
		EXPECT_TRUE(HasFlag(TestEnum::Alpha, flags));
		EXPECT_FALSE(HasFlag(TestEnum::Beta, flags));
		EXPECT_TRUE(HasFlag(TestEnum::Gamma, flags));
		EXPECT_FALSE(HasFlag(TestEnum::Delta, flags));
	}

	// endregion
}}
