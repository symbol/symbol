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

#include "catapult/utils/MacroBasedEnumIncludes.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS MacroBasedEnumTests

	// region output - implicit backing type

	namespace {
// declare and define a GreekLetters enumeration composed of some greek letters with an implicit backing type
#define GREEK_LETTERS_LIST \
	ENUM_VALUE(Alpha) \
	\
	ENUM_VALUE(Beta) \
	\
	ENUM_VALUE(Gamma) \
	\
	ENUM_VALUE(Delta) \
	\
	ENUM_VALUE(Epslion) \
	\
	ENUM_VALUE(Zeta) \
	\
	ENUM_VALUE(Eta)

#define ENUM_VALUE(LABEL) LABEL,
		enum class GreekLetters {
			GREEK_LETTERS_LIST
		};
#undef ENUM_VALUE

#define ENUM_LIST GREEK_LETTERS_LIST
#define DEFINE_ENUM GreekLetters
#include "catapult/utils/MacroBasedEnum.h"
#undef DEFINE_ENUM
#undef ENUM_LIST

		const uint8_t Num_Greek_Letters = []() {
			uint8_t numValues = 0;

#define ENUM_VALUE(LABEL) ++numValues;
			GREEK_LETTERS_LIST
#undef ENUM_VALUE

			return numValues;
		}();
	}

	TEST(TEST_CLASS, KnownValuesAreOutputAsLabel) {
		EXPECT_EQ("Gamma", test::ToString(GreekLetters::Gamma));

		// (0, 1 ... N/2 ... N-2, N-1)
		EXPECT_EQ("Alpha", test::ToString(static_cast<GreekLetters>(0)));
		EXPECT_EQ("Beta", test::ToString(static_cast<GreekLetters>(1)));
		EXPECT_EQ("Delta", test::ToString(static_cast<GreekLetters>(Num_Greek_Letters / 2)));
		EXPECT_EQ("Zeta", test::ToString(static_cast<GreekLetters>(Num_Greek_Letters - 2)));
		EXPECT_EQ("Eta", test::ToString(static_cast<GreekLetters>(Num_Greek_Letters - 1)));
	}

	TEST(TEST_CLASS, UnknownValuesAreOutputAsRawValue) {
		EXPECT_EQ("GreekLetters(0x00000007)", test::ToString(static_cast<GreekLetters>(Num_Greek_Letters)));
		EXPECT_EQ("GreekLetters(0x00000087)", test::ToString(static_cast<GreekLetters>(0x87)));
		EXPECT_EQ("GreekLetters(0x00001234)", test::ToString(static_cast<GreekLetters>(0x1234)));
	}

	// endregion

	// region output - uint8_t backing type

	namespace {
// declare and define a GreekLetters enumeration composed of some greek letters with a uint8_t backing type
#define ENUM_VALUE(LABEL) LABEL,
		enum class GreekLettersByte : uint8_t {
			GREEK_LETTERS_LIST
		};
#undef ENUM_VALUE

#define ENUM_LIST GREEK_LETTERS_LIST
#define DEFINE_ENUM GreekLettersByte
#include "catapult/utils/MacroBasedEnum.h"
#undef DEFINE_ENUM
#undef ENUM_LIST
	}

	TEST(TEST_CLASS, KnownValuesAreOutputAsLabel_UInt8) {
		using GreekLetters = GreekLettersByte;

		// Assert:
		EXPECT_EQ("Gamma", test::ToString(GreekLetters::Gamma));

		// (0, 1 ... N/2 ... N-2, N-1)
		EXPECT_EQ("Alpha", test::ToString(static_cast<GreekLetters>(0)));
		EXPECT_EQ("Beta", test::ToString(static_cast<GreekLetters>(1)));
		EXPECT_EQ("Delta", test::ToString(static_cast<GreekLetters>(Num_Greek_Letters / 2)));
		EXPECT_EQ("Zeta", test::ToString(static_cast<GreekLetters>(Num_Greek_Letters - 2)));
		EXPECT_EQ("Eta", test::ToString(static_cast<GreekLetters>(Num_Greek_Letters - 1)));
	}

	TEST(TEST_CLASS, UnknownValuesAreOutputAsRawValue_UInt8) {
		using GreekLetters = GreekLettersByte;

		// Assert:
		EXPECT_EQ("GreekLettersByte(0x07)", test::ToString(static_cast<GreekLetters>(Num_Greek_Letters)));
		EXPECT_EQ("GreekLettersByte(0x87)", test::ToString(static_cast<GreekLetters>(0x87)));
		EXPECT_EQ("GreekLettersByte(0xFC)", test::ToString(static_cast<GreekLetters>(0xFC)));
	}

	// endregion

	namespace {
// declare and define a VersionParts enumeration composed of version parts
#define VERSION_PARTS_LIST \
	ENUM_VALUE(Major) \
	\
	ENUM_VALUE(Minor) \
	\
	ENUM_VALUE(Build) \
	\
	ENUM_VALUE(Revision)

#define ENUM_VALUE(LABEL) LABEL,
		enum class VersionParts {
			VERSION_PARTS_LIST
		};
#undef ENUM_VALUE

#define ENUM_LIST VERSION_PARTS_LIST
#define DEFINE_ENUM VersionParts
#include "catapult/utils/MacroBasedEnum.h"
#undef DEFINE_ENUM
#undef ENUM_LIST
	}

	TEST(TEST_CLASS, NoConflictsBetweenDifferentEnumTypes) {
		// Assert: overlapping known values
		EXPECT_EQ("Gamma", test::ToString(static_cast<GreekLetters>(2)));
		EXPECT_EQ("Build", test::ToString(static_cast<VersionParts>(2)));

		// Assert: overlapping unknown values
		EXPECT_EQ("GreekLetters(0x00000062)", test::ToString(static_cast<GreekLetters>(0x62)));
		EXPECT_EQ("VersionParts(0x00000062)", test::ToString(static_cast<VersionParts>(0x62)));
	}

	namespace {
// declare and define a GreekLettersExplicit enumeration composed of some greek letters with explicit values
#define GREEK_LETTERS_LIST_EXPLICIT \
	ENUM_VALUE(Alpha, 10) \
	\
	ENUM_VALUE(Beta, 4) \
	\
	ENUM_VALUE(Gamma, 7) \
	\
	ENUM_VALUE(Delta, 18) \
	\
	ENUM_VALUE(Epslion, 9) \
	\
	ENUM_VALUE(Zeta, 2) \
	\
	ENUM_VALUE(Eta, 5)

#define ENUM_VALUE(LABEL, VALUE) LABEL = VALUE,
		enum class GreekLettersExplicit {
			GREEK_LETTERS_LIST_EXPLICIT
		};
#undef ENUM_VALUE

#define ENUM_LIST GREEK_LETTERS_LIST_EXPLICIT
#define EXPLICIT_VALUE_ENUM
#define DEFINE_ENUM GreekLettersExplicit
#include "catapult/utils/MacroBasedEnum.h"
#undef DEFINE_ENUM
#undef EXPLICIT_VALUE_ENUM
#undef ENUM_LIST
	}

	TEST(TEST_CLASS, KnownExplicitValuesAreOutputAsLabel) {
		EXPECT_EQ("Gamma", test::ToString(GreekLetters::Gamma));

		// positional (0, 1 ... N/2 ... N-2, N-1)
		EXPECT_EQ("Alpha", test::ToString(static_cast<GreekLettersExplicit>(10)));
		EXPECT_EQ("Beta", test::ToString(static_cast<GreekLettersExplicit>(4)));
		EXPECT_EQ("Delta", test::ToString(static_cast<GreekLettersExplicit>(18)));
		EXPECT_EQ("Zeta", test::ToString(static_cast<GreekLettersExplicit>(2)));
		EXPECT_EQ("Eta", test::ToString(static_cast<GreekLettersExplicit>(5)));
	}

	TEST(TEST_CLASS, UnknownExplicitValuesAreOutputAsRawValue) {
		EXPECT_EQ("GreekLettersExplicit(0x00000000)", test::ToString(static_cast<GreekLettersExplicit>(0)));
		EXPECT_EQ("GreekLettersExplicit(0x00000003)", test::ToString(static_cast<GreekLettersExplicit>(3)));
		EXPECT_EQ("GreekLettersExplicit(0x00000087)", test::ToString(static_cast<GreekLettersExplicit>(0x87)));
		EXPECT_EQ("GreekLettersExplicit(0x00001234)", test::ToString(static_cast<GreekLettersExplicit>(0x1234)));
	}

	namespace {
// declare and define multiple enums with a single value and varying backing types
#define SINGLE_VALUE_ENUM_LIST \
	ENUM_VALUE(Foo)

#define ENUM_LIST SINGLE_VALUE_ENUM_LIST

#define ENUM_VALUE(LABEL) LABEL,
		enum class SingleValueEnumUInt8 : uint8_t {
			ENUM_LIST
		};
#undef ENUM_VALUE

#define ENUM_VALUE(LABEL) LABEL,
		enum class SingleValueEnumImplicit {
			ENUM_LIST
		};
#undef ENUM_VALUE

#define ENUM_VALUE(LABEL) LABEL,
		enum class SingleValueEnumUInt64 : uint64_t {
			ENUM_LIST
		};
#undef ENUM_VALUE

// define and use the operator<< to avoid compiler warnings
#define DEFINE_ENUM SingleValueEnumUInt8
#include "catapult/utils/MacroBasedEnum.h"
#undef DEFINE_ENUM

#define DEFINE_ENUM SingleValueEnumImplicit
#include "catapult/utils/MacroBasedEnum.h"
#undef DEFINE_ENUM

#define DEFINE_ENUM SingleValueEnumUInt64
#include "catapult/utils/MacroBasedEnum.h"
#undef DEFINE_ENUM

#undef ENUM_LIST

		template<typename TEnum, typename TExpectedBackingType>
		void AssertHasBackingType() {
			// Act:
			using ActualBackingType = std::underlying_type_t<TEnum>;
			auto areTypesSame = std::is_same_v<TExpectedBackingType, ActualBackingType>;

			// Assert:
			EXPECT_TRUE(areTypesSame) << "sizeof(TEnum): " << sizeof(TEnum) << " " << TEnum::Foo;
		}
	}

	TEST(TEST_CLASS, CanSpecifyImplicitEnumBackingType) {
		AssertHasBackingType<SingleValueEnumImplicit, int32_t>();
	}

	TEST(TEST_CLASS, CanSpecifyExplicitEnumBackingType) {
		AssertHasBackingType<SingleValueEnumUInt8, uint8_t>();
		AssertHasBackingType<SingleValueEnumUInt64, uint64_t>();
	}
}}
