#include "src/model/NameChecker.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	namespace {
		auto GenerateValidName(size_t size) {
			static constexpr char Valid_Alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789";

			std::vector<uint8_t> name(size);
			std::generate(name.begin(), name.end(), []() {
				return Valid_Alphabet[test::Random() % (CountOf(Valid_Alphabet) - 1)];
			});
			return name;
		}
	}

	TEST(NameCheckerTests, NameIsValidWhenAllCharactersAreAlphaNumeric_Random) {
		// Arrange:
		auto name = GenerateValidName(7);

		// Assert:
		EXPECT_TRUE(IsValidName(name.data(), name.size()));
	}

	TEST(NameCheckerTests, NameIsInvalidWhenNoCharactersAreAlphaNumeric_Random) {
		// Arrange:
		auto name = GenerateValidName(7);
		for (auto& ch : name)
			ch ^= 0xFF;

		// Assert:
		EXPECT_FALSE(IsValidName(name.data(), name.size()));
	}

	TEST(NameCheckerTests, NameIsInvalidWhenSingleCharactersIsNotAlphaNumeric_Random) {
		// Arrange:
		auto name = GenerateValidName(7);
		name[3] ^= 0xFF;

		// Assert:
		EXPECT_FALSE(IsValidName(name.data(), name.size()));
	}

	namespace {
		bool IsValidName(const std::string& str) {
			return model::IsValidName(reinterpret_cast<const uint8_t*>(str.data()), str.size());
		}
	}

	TEST(NameCheckerTests, NameIsValidWhenAllCharactersAreAlphaNumeric) {
		// Assert:
		for (const auto& name : { "a", "be", "cat", "doom" })
			EXPECT_TRUE(IsValidName(name));
	}

	TEST(NameCheckerTests, NameIsValidWhenItContainsSeparator) {
		// Assert:
		for (const auto& name : { "al-ce", "al_ce", "alice-", "alice_" })
			EXPECT_TRUE(IsValidName(name));
	}

	TEST(NameCheckerTests, NameIsInvalidWhenItStartsWithSeparator) {
		// Assert:
		for (const auto& name : { "-alice", "_alice" })
			EXPECT_FALSE(IsValidName(name));
	}

	TEST(NameCheckerTests, NameIsInvalidWhenItContainsInvalidCharacters) {
		// Assert:
		for (const auto& name : { "al.ce", "alIce", "al ce", "al@ce", "al#ce" })
			EXPECT_FALSE(IsValidName(name));
	}
}}
