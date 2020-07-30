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

#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"
#include <array>
#include <vector>

namespace catapult { namespace utils {

#define TEST_CLASS HexParserTests

	namespace {
		// region Parse traits

		struct ParseByteTraits {
			static uint8_t Parse(char ch1, char ch2) {
				return ParseByte(ch1, ch2);
			}

			static void AssertBadParse(char ch1, char ch2) {
				EXPECT_THROW(ParseByte(ch1, ch2), catapult_invalid_argument);
			}
		};

		struct ParseTraits {
			template<typename TContainer>
			static void ParseString(const char* const pHexData, size_t dataSize, TContainer& outputContainer) {
				ParseHexStringIntoContainer(pHexData, dataSize, outputContainer);
			}

			template<typename TContainer>
			static void AssertBadParse(const char* const pHexData, size_t dataSize, TContainer& outputContainer) {
				EXPECT_THROW(ParseHexStringIntoContainer(pHexData, dataSize, outputContainer), catapult_invalid_argument);
			}
		};

		struct ParseByteArrayTraits {
			template<typename TContainer>
			struct ByteArrayTag {
				static constexpr size_t Size = std::tuple_size_v<TContainer>;
			};

			template<typename TContainer>
			static void ParseString(const char* const pHexData, size_t dataSize, TContainer& outputContainer) {
				using ByteArrayContainer = ByteArray<ByteArrayTag<TContainer>>;
				auto hexString = std::string(pHexData, dataSize);
				auto byteArray = ParseByteArray<ByteArrayContainer>(hexString);
				std::copy(byteArray.cbegin(), byteArray.cend(), outputContainer.begin());
			}

			template<typename TContainer>
			static void AssertBadParse(const char* const pHexData, size_t dataSize, TContainer&) {
				using ByteArrayContainer = ByteArray<ByteArrayTag<TContainer>>;
				auto hexString = std::string(pHexData, dataSize);
				EXPECT_THROW(ParseByteArray<ByteArrayContainer>(hexString), catapult_invalid_argument);
			}
		};

		// endregion

		// region TryParse traits

		struct TryParseByteTraits {
			static uint8_t Parse(char ch1, char ch2) {
				uint8_t by;
				EXPECT_TRUE(TryParseByte(ch1, ch2, by));
				return by;
			}

			static void AssertBadParse(char ch1, char ch2) {
				uint8_t by;
				EXPECT_FALSE(TryParseByte(ch1, ch2, by));
			}
		};

		struct TryParseTraits {
			template<typename TContainer>
			static void ParseString(const char* const pHexData, size_t dataSize, TContainer& outputContainer) {
				EXPECT_TRUE(TryParseHexStringIntoContainer(pHexData, dataSize, outputContainer));
			}

			template<typename TContainer>
			static void AssertBadParse(const char* const pHexData, size_t dataSize, TContainer& outputContainer) {
				EXPECT_FALSE(TryParseHexStringIntoContainer(pHexData, dataSize, outputContainer));
			}
		};

		// endregion
	}

#define PARSE_BYTE_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ParseByteTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Try) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TryParseByteTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define PARSE_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ParseTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ByteArray) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ParseByteArrayTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Try) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TryParseTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	PARSE_BYTE_TRAITS_BASED_TEST(CanConvertAllValidHexCharCombinationsToByte) {
		// Arrange:
		std::vector<std::pair<char, uint8_t>> charToValueMappings;
		for (char ch = '0'; ch <= '9'; ++ch) charToValueMappings.push_back(std::make_pair(ch, static_cast<uint8_t>(ch - '0')));
		for (char ch = 'a'; ch <= 'f'; ++ch) charToValueMappings.push_back(std::make_pair(ch, static_cast<uint8_t>(ch - 'a' + 10)));
		for (char ch = 'A'; ch <= 'F'; ++ch) charToValueMappings.push_back(std::make_pair(ch, static_cast<uint8_t>(ch - 'A' + 10)));

		// Act:
		auto numTests = 0;
		for (const auto& pair1 : charToValueMappings) {
			for (const auto& pair2 : charToValueMappings) {
				// Act:
				uint8_t byte = TTraits::Parse(pair1.first, pair2.first);

				// Assert:
				auto expected = static_cast<uint8_t>(pair1.second * 16 + pair2.second);
				EXPECT_EQ(expected, byte) << "input: " << pair1.first << pair2.first;
				++numTests;
			}
		}

		// Sanity:
		EXPECT_EQ(22 * 22, numTests);
	}

	PARSE_BYTE_TRAITS_BASED_TEST(CannotConvertInvalidHexCharsToByte) {
		TTraits::AssertBadParse('G', '6');
		TTraits::AssertBadParse('7', 'g');
		TTraits::AssertBadParse('*', '8');
		TTraits::AssertBadParse('9', '!');
	}

	PARSE_TRAITS_BASED_TEST(CanParseValidHexStringIntoContainer) {
		// Act:
		using ArrayType = std::array<uint8_t, 6>;
		ArrayType array;
		TTraits::ParseString("026ee415fc15", 12, array);

		// Assert:
		ArrayType expected{ { 0x02, 0x6E, 0xE4, 0x15, 0xFC, 0x15 } };
		EXPECT_EQ(expected, array);
	}

	PARSE_TRAITS_BASED_TEST(CanParseValidHexStringContainingAllValidHexCharsIntoContainer) {
		// Act:
		using ArrayType = std::array<uint8_t, 11>;
		ArrayType array;
		TTraits::ParseString("abcdef0123456789ABCDEF", 22, array);

		// Assert:
		ArrayType expected{ { 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF } };
		EXPECT_EQ(expected, array);
	}

	PARSE_TRAITS_BASED_TEST(CannotParseHexStringWithInvalidHexCharsIntoContainer) {
		std::array<uint8_t, 11> array;
		TTraits::AssertBadParse("abcdef012345G789ABCDEF", 22, array);
	}

	PARSE_TRAITS_BASED_TEST(CannotParseValidHexStringWithInvalidSizeIntoContainer) {
		// Assert: the only allowable size is 2 * 10 == 20
		std::array<uint8_t, 10> array;
		TTraits::AssertBadParse("abcdef0123456789ABCDEF", 18, array);
		TTraits::AssertBadParse("abcdef0123456789ABCDEF", 19, array);
		TTraits::AssertBadParse("abcdef0123456789ABCDEF", 21, array);
		TTraits::AssertBadParse("abcdef0123456789ABCDEF", 22, array);
	}
}}
