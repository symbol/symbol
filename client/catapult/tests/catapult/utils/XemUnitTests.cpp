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

#include "catapult/utils/XemUnit.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS XemUnitTests

	// region constructor

	TEST(TEST_CLASS, CanCreateDefaultUnit) {
		// Arrange:
		XemUnit unit;

		// Act + Assert:
		EXPECT_EQ(XemAmount(0), unit.xem());
		EXPECT_EQ(Amount(0), unit.microxem());
		EXPECT_FALSE(unit.isFractional());
	}

	TEST(TEST_CLASS, CanCreateUnitFromAmount) {
		// Arrange:
		XemUnit unit(Amount(123'000'000));

		// Act + Assert:
		EXPECT_EQ(XemAmount(123), unit.xem());
		EXPECT_EQ(Amount(123'000'000), unit.microxem());
		EXPECT_FALSE(unit.isFractional());
	}

	TEST(TEST_CLASS, CanCreateUnitFromXemAmount) {
		// Arrange:
		XemUnit unit(XemAmount(123'000'000));

		// Act + Assert:
		EXPECT_EQ(XemAmount(123'000'000), unit.xem());
		EXPECT_EQ(Amount(123'000'000'000'000), unit.microxem());
		EXPECT_FALSE(unit.isFractional());
	}

	TEST(TEST_CLASS, CanCreateUnitFromFractionalAmount) {
		// Arrange:
		XemUnit unit(Amount(123'789'432));

		// Act + Assert:
		EXPECT_EQ(XemAmount(123), unit.xem());
		EXPECT_EQ(Amount(123'789'432), unit.microxem());
		EXPECT_TRUE(unit.isFractional());
	}

	// endregion

	// region copy + assign

	TEST(TEST_CLASS, CanCopyAssign) {
		// Arrange:
		XemUnit unit(XemAmount(123));
		XemUnit copy(XemAmount(456));

		// Act:
		const auto& assignResult = (copy = unit);

		// Assert:
		EXPECT_EQ(XemAmount(123), unit.xem());
		EXPECT_EQ(XemAmount(123), copy.xem());
		EXPECT_EQ(&copy, &assignResult);
	}

	TEST(TEST_CLASS, CanCopyConstruct) {
		// Act:
		XemUnit unit(XemAmount(123));
		XemUnit copy(unit);

		// Assert:
		EXPECT_EQ(XemAmount(123), unit.xem());
		EXPECT_EQ(XemAmount(123), copy.xem());
	}

	TEST(TEST_CLASS, CanMoveAssign) {
		// Arrange:
		XemUnit unit(XemAmount(123));
		XemUnit copy(XemAmount(456));

		// Act:
		const auto& assignResult = (copy = std::move(unit));

		// Assert:
		EXPECT_EQ(XemAmount(123), copy.xem());
		EXPECT_EQ(&copy, &assignResult);
	}

	TEST(TEST_CLASS, CanMoveConstruct) {
		// Act:
		XemUnit unit(XemAmount(123));
		XemUnit copy(std::move(unit));

		// Assert:
		EXPECT_EQ(XemAmount(123), copy.xem());
	}

	// endregion

	// region custom assign

	TEST(TEST_CLASS, CanAssignUnitFromAmount) {
		// Act:
		XemUnit unit;
		const auto& assignResult = (unit = Amount(123'000'000));

		// Assert:
		EXPECT_EQ(XemAmount(123), unit.xem());
		EXPECT_EQ(Amount(123'000'000), unit.microxem());
		EXPECT_FALSE(unit.isFractional());
		EXPECT_EQ(&unit, &assignResult);
	}

	TEST(TEST_CLASS, CanAssignUnitFromXemAmount) {
		// Act:
		XemUnit unit;
		const auto& assignResult = (unit = XemAmount(123'000'000));

		// Assert:
		EXPECT_EQ(XemAmount(123'000'000), unit.xem());
		EXPECT_EQ(Amount(123'000'000'000'000), unit.microxem());
		EXPECT_FALSE(unit.isFractional());
		EXPECT_EQ(&unit, &assignResult);
	}

	TEST(TEST_CLASS, CanAssignUnitFromFractionalAmount) {
		// Arrange:
		XemUnit unit;
		const auto& assignResult = (unit = Amount(123'789'432));

		// Assert:
		EXPECT_EQ(XemAmount(123), unit.xem());
		EXPECT_EQ(Amount(123'789'432), unit.microxem());
		EXPECT_TRUE(unit.isFractional());
		EXPECT_EQ(&unit, &assignResult);
	}

	// endregion

	// region comparison operators

	namespace {
		std::vector<XemUnit> GenerateIncreasingValues() {
			return { XemUnit(Amount(123)), XemUnit(Amount(642)), XemUnit(Amount(989)) };
		}
	}

	DEFINE_EQUALITY_TESTS(TEST_CLASS, GenerateIncreasingValues())

	// endregion

	// region to string

	TEST(TEST_CLASS, CanOutputWholeUnitAmount) {
		// Arrange:
		XemUnit unit(XemAmount(123));

		// Act:
		auto str = test::ToString(unit);

		// Assert:
		EXPECT_EQ("123", str);
	}

	TEST(TEST_CLASS, CanOutputFractionalUnitAmount) {
		// Arrange:
		XemUnit unit(Amount(123'009'432));

		// Act:
		auto str = test::ToString(unit);

		// Assert:
		EXPECT_EQ("123.009432", str);
	}

	TEST(TEST_CLASS, OutputFormattingChangesDoNotLeak) {
		// Arrange:
		std::ostringstream out;
		out.flags(std::ios::hex | std::ios::uppercase);
		out.fill('~');

		// Act:
		out << std::setw(4) << 0xAB << " " << XemUnit(Amount(123'009'432)) << " " << std::setw(4) << 0xCD;
		auto actual = out.str();

		// Assert:
		EXPECT_EQ("~~AB 123.009432 ~~CD", actual);
	}

	// endregion

	// region TryParseValue

	TEST(TEST_CLASS, TryParseValueFailsWhenParsingInvalidAmount) {
		test::AssertFailedParse("45Z'000'000", XemUnit(), [](const auto& str, auto& parsedValue) {
			return TryParseValue(str, parsedValue);
		});
	}

	TEST(TEST_CLASS, TryParseValueFailsWhenParsingFractionalAmount) {
		test::AssertFailedParse("450'000'001", XemUnit(), [](const auto& str, auto& parsedValue) {
			return TryParseValue(str, parsedValue);
		});
	}

	TEST(TEST_CLASS, TryParseValueSucceedsWhenParsingWholeUnitAmount) {
		test::AssertParse("450'000'000", XemUnit(XemAmount(450)), [](const auto& str, auto& parsedValue) {
			return TryParseValue(str, parsedValue);
		});
	}

	// endregion
}}
