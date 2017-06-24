#include "catapult/utils/DiagnosticCounterId.h"
#include "tests/TestHarness.h"
#include <cmath>

namespace catapult { namespace utils {

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

	TEST(DiagnosticCounterIdTests, CanCreateDefaultId) {
		// Act:
		DiagnosticCounterId id;

		// Assert:
		EXPECT_EQ("", id.name());
		EXPECT_EQ(0u, id.value());
	}

	TEST(DiagnosticCounterIdTests, CanCreateAroundEmptyString) {
		// Assert:
		AssertCanCreateIdFromString("", 0);
	}

	TEST(DiagnosticCounterIdTests, CanCreateAroundZeroValue) {
		// Assert:
		AssertCanCreateIdFromValue(0, "");
	}

	// endregion

	// region known / max

	TEST(DiagnosticCounterIdTests, CanCreateAroundKnownString) {
		// Assert:
		AssertCanCreateIdFromString("Z CAT", 26u * 27 * 27 * 27 * 27 + 3u * 27 * 27 + 1u * 27 + 20u);
	}

	TEST(DiagnosticCounterIdTests, CanCreateAroundKnownValue) {
		// Assert:
		AssertCanCreateIdFromValue(26u * 27 * 27 * 27 * 27 + 3u * 27 * 27 + 1u * 27 + 20u, "Z CAT");
	}

	TEST(DiagnosticCounterIdTests, CanCreateAroundMaxString) {
		// Assert:
		AssertCanCreateIdFromString(std::string(DiagnosticCounterId::Max_Counter_Name_Size, 'Z'), Max_Id_Value);
	}

	TEST(DiagnosticCounterIdTests, CanCreateAroundMaxValue) {
		// Assert:
		AssertCanCreateIdFromValue(Max_Id_Value, std::string(DiagnosticCounterId::Max_Counter_Name_Size, 'Z'));
	}

	TEST(DiagnosticCounterIdTests, MaxValueConstantIsCorrect) {
		// Assert:
		auto maxValue = std::numeric_limits<decltype(DiagnosticCounterId().value())>::max();
		EXPECT_LT(std::pow(27, DiagnosticCounterId::Max_Counter_Name_Size), maxValue);
		EXPECT_GT(std::pow(27, DiagnosticCounterId::Max_Counter_Name_Size + 1), maxValue);
	}

	// endregion

	// region single char strings

	TEST(DiagnosticCounterIdTests, CanCreateAroundSingleCharStrings) {
		// Assert:
		for (auto ch = 'A'; ch <= 'Z'; ++ch)
			AssertCanCreateIdFromString(std::string{ ch }, static_cast<uint8_t>(ch - 'A') + 1);
	}

	TEST(DiagnosticCounterIdTests, CanCreateAroundSingleCharValues) {
		// Assert:
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

	TEST(DiagnosticCounterIdTests, CanRoundtripArbitraryStrings) {
		// Arrange: try to round trip some candidate strings
		for (const auto& name : { "W SPC", "X", "DD  SPC", "ACT RDR", "WOSPC", "ABC", "ZY BA", "CAT", "ABCDEFGH", "MAXLENAA" })
			AssertCanRoundtripName(name);
	}

	TEST(DiagnosticCounterIdTests, CanRoundtripMaxLengthSingleCharStrings) {
		// Assert:
		for (auto ch = 'A'; ch <= 'Z'; ++ch)
			AssertCanRoundtripName(std::string(DiagnosticCounterId::Max_Counter_Name_Size, ch));
	}

	// endregion

	// region invalid strings

	namespace {
		void AssertInvalidName(const std::string& name) {
			// Assert:
			EXPECT_THROW(DiagnosticCounterId id(name), catapult_invalid_argument) << name;
		}
	}

	TEST(DiagnosticCounterIdTests, CannotCreateAroundStringTooLong) {
		// Assert:
		for (const auto& name : { "ABCDEFGHIJKLMN", "ABC EFG IJKLMN", "ABCD    IJKLMN", "ABCDEFGHIJKLMNOP" })
			AssertInvalidName(name);

		AssertInvalidName(std::string(DiagnosticCounterId::Max_Counter_Name_Size + 1, ' '));
		AssertInvalidName(std::string(DiagnosticCounterId::Max_Counter_Name_Size + 1, 'Z'));
	}

	TEST(DiagnosticCounterIdTests, CannotCreateAroundStringWithInvalidChars) {
		// Assert:
		for (const auto& name : { "-", "T@SK", "ABCDEFG!", "$ABC", "ABC$" })
			AssertInvalidName(name);
	}

	// region whitespace

	TEST(DiagnosticCounterIdTests, CannotCreateAroundWhitespaceString) {
		// Assert:
		for (auto size : { 1u, 4u, DiagnosticCounterId::Max_Counter_Name_Size })
			AssertInvalidName(std::string(size, ' '));
	}

	TEST(DiagnosticCounterIdTests, CannotCreateAroundStringWithLeadingWhitespace) {
		// Assert:
		AssertInvalidName("   J");
	}

	TEST(DiagnosticCounterIdTests, CannotCreateAroundStringWithTrailingWhitespace) {
		// Assert:
		AssertInvalidName("J   ");
	}

	// endregion

	// endregion

	// region invalid values

	namespace {
		void AssertInvalidValue(uint64_t value) {
			// Assert:
			EXPECT_THROW(DiagnosticCounterId id(value), catapult_invalid_argument) << value;
		}
	}

	TEST(DiagnosticCounterIdTests, CannotCreateIdAroundValueGreaterThanMax) {
		// Assert:
		AssertInvalidValue(Max_Id_Value + 1);
		AssertInvalidValue(std::numeric_limits<uint64_t>::max());
	}

	// endregion
}}
