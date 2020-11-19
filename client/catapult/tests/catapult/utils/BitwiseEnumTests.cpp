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

	// region or (equals) operator

	TEST(TEST_CLASS, CanOrBitwiseFlags) {
		EXPECT_EQ(0x05, utils::to_underlying_type(TestEnum::Alpha | TestEnum::Gamma));
	}

	TEST(TEST_CLASS, OrIsIdempotent) {
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
		for (auto flag : { TestEnum::Alpha, TestEnum::Beta, TestEnum::Gamma, TestEnum::Delta, TestEnum::All })
			EXPECT_TRUE(HasFlag(flag, TestEnum::All)) << "flag " << utils::to_underlying_type(flag);
	}

	TEST(TEST_CLASS, HasFlagReturnsFalseWhenAllFlagsAreUnset) {
		for (auto flag : { TestEnum::Alpha, TestEnum::Beta, TestEnum::Gamma, TestEnum::Delta, TestEnum::All })
			EXPECT_FALSE(HasFlag(flag, TestEnum::None)) << "flag " << utils::to_underlying_type(flag);
	}

	TEST(TEST_CLASS, HasFlagAlwaysReturnsTrueWhenTestedFlagIsZero) {
		for (auto flags : { TestEnum::None, TestEnum::Alpha, TestEnum::Beta, TestEnum::Gamma, TestEnum::Delta, TestEnum::All })
			EXPECT_TRUE(HasFlag(TestEnum::None, flags)) << "flags " << utils::to_underlying_type(flags);
	}

	TEST(TEST_CLASS, HasFlagReturnsTrueOnlyWhenFlagIsSet) {
		// Arrange:
		auto flags = TestEnum::Alpha | TestEnum::Gamma;

		// Act + Assert:
		EXPECT_TRUE(HasFlag(TestEnum::Alpha, flags));
		EXPECT_FALSE(HasFlag(TestEnum::Beta, flags));
		EXPECT_TRUE(HasFlag(TestEnum::Gamma, flags));
		EXPECT_FALSE(HasFlag(TestEnum::Delta, flags));
	}

	// endregion

	// region has single flag

	TEST(TEST_CLASS, HasSingleFlagReturnsFalseWhenNoBitsAreSet) {
		EXPECT_FALSE(HasSingleFlag(TestEnum::None));
	}

	TEST(TEST_CLASS, HasSingleFlagReturnsTrueWhenSingleBitIsSet) {
		for (auto flag : { TestEnum::Alpha, TestEnum::Beta, TestEnum::Gamma, TestEnum::Delta })
			EXPECT_TRUE(HasSingleFlag(flag)) << "flag " << utils::to_underlying_type(flag);
	}

	TEST(TEST_CLASS, HasSingleFlagReturnsFalseWhenMultipleBitsAreSet) {
		for (auto flag1 : { TestEnum::Alpha, TestEnum::Beta }) {
			for (auto flag2 : { TestEnum::Gamma, TestEnum::Delta }) {
				EXPECT_FALSE(HasSingleFlag(flag1 | flag2))
						<< "flag1 " << utils::to_underlying_type(flag1)
						<< ", flag2 " << utils::to_underlying_type(flag2);
			}
		}
	}

	TEST(TEST_CLASS, HasSingleFlagReturnsFalseWhenAllBitsAreSet) {
		EXPECT_FALSE(HasSingleFlag(TestEnum::All));
	}

	// endregion
}}
