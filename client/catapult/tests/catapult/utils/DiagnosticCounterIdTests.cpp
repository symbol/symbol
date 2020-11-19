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

#include "catapult/utils/DiagnosticCounterId.h"
#include "tests/TestHarness.h"
#include <cmath>

namespace catapult { namespace utils {

#define TEST_CLASS DiagnosticCounterIdTests

	namespace {
		constexpr auto Max_Id_Value = 4052555153018976266ULL;

		void AssertCanCreateIdFromString(const std::string& name, uint64_t expectedValue) {
			// Act:
			DiagnosticCounterId id(name);

			// Assert:
			std::string message = "name " + name + ", value " + std::to_string(expectedValue);
			EXPECT_EQ(name, id.name()) << message;
			EXPECT_EQ(expectedValue, id.value()) << message;
		}

		void AssertCanCreateIdFromValue(uint64_t value, const std::string& expectedName) {
			// Act:
			DiagnosticCounterId id(value);

			// Assert:
			std::string message = "name " + expectedName + ", value " + std::to_string(value);
			EXPECT_EQ(expectedName, id.name()) << message;
			EXPECT_EQ(value, id.value()) << message;
		}
	}

	// region empty

	TEST(TEST_CLASS, CanCreateDefaultId) {
		// Act:
		DiagnosticCounterId id;

		// Assert:
		EXPECT_EQ("", id.name());
		EXPECT_EQ(0u, id.value());
	}

	TEST(TEST_CLASS, CanCreateAroundEmptyString) {
		AssertCanCreateIdFromString("", 0);
	}

	TEST(TEST_CLASS, CanCreateAroundZeroValue) {
		AssertCanCreateIdFromValue(0, "");
	}

	// endregion

	// region known / max

	TEST(TEST_CLASS, CanCreateAroundKnownString) {
		AssertCanCreateIdFromString("Z CAT", 26u * 27 * 27 * 27 * 27 + 3 * 27 * 27 + 1 * 27 + 20);
	}

	TEST(TEST_CLASS, CanCreateAroundKnownValue) {
		AssertCanCreateIdFromValue(26u * 27 * 27 * 27 * 27 + 3 * 27 * 27 + 1 * 27 + 20, "Z CAT");
	}

	TEST(TEST_CLASS, CanCreateAroundMaxString) {
		AssertCanCreateIdFromString(std::string(DiagnosticCounterId::Max_Counter_Name_Size, 'Z'), Max_Id_Value);
	}

	TEST(TEST_CLASS, CanCreateAroundMaxValue) {
		AssertCanCreateIdFromValue(Max_Id_Value, std::string(DiagnosticCounterId::Max_Counter_Name_Size, 'Z'));
	}

	TEST(TEST_CLASS, MaxValueConstantIsCorrect) {
		auto maxValue = std::numeric_limits<decltype(DiagnosticCounterId().value())>::max();
		EXPECT_LT(std::pow(27, DiagnosticCounterId::Max_Counter_Name_Size), maxValue);
		EXPECT_GT(std::pow(27, DiagnosticCounterId::Max_Counter_Name_Size + 1), maxValue);
	}

	// endregion

	// region single char strings

	TEST(TEST_CLASS, CanCreateAroundSingleCharStrings) {
		for (auto ch = 'A'; ch <= 'Z'; ++ch)
			AssertCanCreateIdFromString(std::string{ ch }, static_cast<uint8_t>(ch - 'A') + 1);
	}

	TEST(TEST_CLASS, CanCreateAroundSingleCharValues) {
		for (auto ch = 'A'; ch <= 'Z'; ++ch)
			AssertCanCreateIdFromValue(static_cast<uint8_t>(ch - 'A') + 1, std::string{ ch });
	}

	// endregion

	// region roundtrip

	namespace {
		void AssertCanRoundtripName(const std::string& name) {
			// Act:
			auto id = DiagnosticCounterId(DiagnosticCounterId(name).value());

			// Assert:
			EXPECT_EQ(name, id.name());
		}
	}

	TEST(TEST_CLASS, CanRoundtripArbitraryStrings) {
		// Arrange: try to roundtrip some candidate strings
		for (const auto& name : { "W SPC", "X", "DD  SPC", "ACT RDR", "WOSPC", "ABC", "ZY BA", "CAT", "ABCDEFGH", "MAXLENAA" })
			AssertCanRoundtripName(name);
	}

	TEST(TEST_CLASS, CanRoundtripMaxLengthSingleCharStrings) {
		for (auto ch = 'A'; ch <= 'Z'; ++ch)
			AssertCanRoundtripName(std::string(DiagnosticCounterId::Max_Counter_Name_Size, ch));
	}

	// endregion

	// region invalid strings

	namespace {
		void AssertInvalidName(const std::string& name) {
			// Act + Assert:
			EXPECT_THROW(DiagnosticCounterId id(name), catapult_invalid_argument) << name;
		}
	}

	TEST(TEST_CLASS, CannotCreateAroundStringTooLong) {
		for (const auto& name : { "ABCDEFGHIJKLMN", "ABC EFG IJKLMN", "ABCD    IJKLMN", "ABCDEFGHIJKLMNOP" })
			AssertInvalidName(name);

		AssertInvalidName(std::string(DiagnosticCounterId::Max_Counter_Name_Size + 1, ' '));
		AssertInvalidName(std::string(DiagnosticCounterId::Max_Counter_Name_Size + 1, 'Z'));
	}

	TEST(TEST_CLASS, CannotCreateAroundStringWithInvalidChars) {
		for (const auto& name : { "-", "T@SK", "ABCDEFG!", "$ABC", "ABC$" })
			AssertInvalidName(name);
	}

	// endregion

	// region invalid strings - whitespace

	TEST(TEST_CLASS, CannotCreateAroundWhitespaceString) {
		for (auto size : { 1u, 4u, DiagnosticCounterId::Max_Counter_Name_Size })
			AssertInvalidName(std::string(size, ' '));
	}

	TEST(TEST_CLASS, CannotCreateAroundStringWithLeadingWhitespace) {
		AssertInvalidName("   J");
	}

	TEST(TEST_CLASS, CannotCreateAroundStringWithTrailingWhitespace) {
		AssertInvalidName("J   ");
	}

	// endregion

	// region invalid values

	namespace {
		void AssertInvalidValue(uint64_t value) {
			// Act + Assert:
			EXPECT_THROW(DiagnosticCounterId id(value), catapult_invalid_argument) << value;
		}
	}

	TEST(TEST_CLASS, CannotCreateIdAroundValueGreaterThanMax) {
		AssertInvalidValue(Max_Id_Value + 1);
		AssertInvalidValue(std::numeric_limits<uint64_t>::max());
	}

	// endregion
}}
