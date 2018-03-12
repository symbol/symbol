#include "src/model/NameChecker.h"
#include "tests/test/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS NameCheckerTests

	namespace {
		bool IsValidName(const std::string& str) {
			return model::IsValidName(reinterpret_cast<const uint8_t*>(str.data()), str.size());
		}
	}

	TEST(TEST_CLASS, NameIsValidWhenAllCharactersAreAlphaNumeric_Random) {
		// Arrange:
		auto name = test::GenerateValidName(7);

		// Assert:
		EXPECT_TRUE(IsValidName(name));
	}

	TEST(TEST_CLASS, NameIsInvalidWhenNoCharactersAreAlphaNumeric_Random) {
		// Arrange:
		auto name = test::GenerateValidName(7);
		for (auto& ch : name)
			ch ^= 0xFF;

		// Assert:
		EXPECT_FALSE(IsValidName(name));
	}

	TEST(TEST_CLASS, NameIsInvalidWhenSingleCharactersIsNotAlphaNumeric_Random) {
		// Arrange:
		auto name = test::GenerateValidName(7);
		name[3] ^= 0xFF;

		// Assert:
		EXPECT_FALSE(IsValidName(name));
	}

	TEST(TEST_CLASS, NameIsValidWhenAllCharactersAreAlphaNumeric) {
		// Assert:
		for (const auto& name : { "a", "be", "cat", "doom" })
			EXPECT_TRUE(IsValidName(name));
	}

	TEST(TEST_CLASS, NameIsValidWhenItContainsSeparator) {
		// Assert:
		for (const auto& name : { "al-ce", "al_ce", "alice-", "alice_" })
			EXPECT_TRUE(IsValidName(name));
	}

	TEST(TEST_CLASS, NameIsInvalidWhenItStartsWithSeparator) {
		// Assert:
		for (const auto& name : { "-alice", "_alice" })
			EXPECT_FALSE(IsValidName(name));
	}

	TEST(TEST_CLASS, NameIsInvalidWhenItContainsInvalidCharacters) {
		// Assert:
		for (const auto& name : { "al.ce", "alIce", "al ce", "al@ce", "al#ce" })
			EXPECT_FALSE(IsValidName(name));
	}
}}
