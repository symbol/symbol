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

#include "catapult/utils/Base32.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS Base32Tests

	// region Base32Encode

	namespace {
		const char* Decoded_Set[] = {
			"68BA9E8D1AA4502E1F73DA19784B5D7DA16CA1E4AF895FAC12",
			"684C2605E5B366BB94BC30755EC9F50D74E80FC9283D20E283",
			"68D7B09A14BEA7CE060E71C0FA9AC9B4226DE167013DE10B3D",
			"686C44C024F1089669F53C45AC6D62CC17A0D9CBA67A6205E6",
			"98A0FE84BBFC5EEE7CADC2B12F790DAA4A7A9505096E674FAB"
		};

		constexpr size_t Decoded_String_Size = 25;

		const char* Encoded_Set[] = {
			"NC5J5DI2URIC4H3T3IMXQS25PWQWZIPEV6EV7LAS",
			"NBGCMBPFWNTLXFF4GB2V5SPVBV2OQD6JFA6SBYUD",
			"NDL3BGQUX2T44BQOOHAPVGWJWQRG3YLHAE66CCZ5",
			"NBWEJQBE6EEJM2PVHRC2Y3LCZQL2BWOLUZ5GEBPG",
			"TCQP5BF37RPO47FNYKYS66INVJFHVFIFBFXGOT5L"
		};

		constexpr size_t Encoded_String_Size = 40;

		std::vector<uint8_t> Base32Decode(const RawString& encodedData) {
			std::vector<uint8_t> decodedVector;
			decodedVector.resize(GetDecodedDataSize(encodedData.Size));
			Base32Decode(encodedData, decodedVector);
			return decodedVector;
		}
	}

	TEST(TEST_CLASS, GetEncodedDataSizeReturnsCorrectSizes) {
		EXPECT_EQ(0u, GetEncodedDataSize(0));
		EXPECT_EQ(8u, GetEncodedDataSize(5));
		EXPECT_EQ(16u, GetEncodedDataSize(10));
		EXPECT_EQ(16u, GetEncodedDataSize(14));
		EXPECT_EQ(24u, GetEncodedDataSize(15));
		EXPECT_EQ(24u, GetEncodedDataSize(16));
	}

	TEST(TEST_CLASS, EncodeEmptyByteArrayReturnsEmptyString) {
		// Act:
		auto buffer = test::HexStringToVector("");
		auto encoded = Base32Encode(buffer);

		// Assert:
		EXPECT_EQ("", encoded);
	}

	TEST(TEST_CLASS, EncodeSampleTestVectors) {
		EXPECT_EQ(CountOf(Decoded_Set), CountOf(Encoded_Set));
		for (size_t i = 0; i < CountOf(Decoded_Set); ++i) {
			auto buffer = test::HexStringToVector(Decoded_Set[i]);

			// Act:
			auto actual = Base32Encode(buffer);

			// Assert:
			EXPECT_EQ(Encoded_Set[i], actual);
		}
	}

	TEST(TEST_CLASS, EncodeThrowsWhenArraySizeIsNotAMultipleOfFive) {
		for (auto i = 2u; i < 10; i += 2) {
			// Arrange:
			auto buffer = test::HexStringToVector(std::string(i, '1'));

			// Act + Assert:
			EXPECT_THROW(Base32Encode(buffer), catapult_runtime_error);
		}
	}

	TEST(TEST_CLASS, EncodeAcceptsAllByteValues) {
		// Arrange:
		std::vector<uint8_t> buffer;
		buffer.resize(260); // must be multiple of 5
		for (auto i = 0u; i < 260; ++i)
			buffer[i] = static_cast<uint8_t>(i);

		// Act:
		std::string encoded = Base32Encode(buffer);

		// Assert:
		std::string expected =
			"AAAQEAYEAUDAOCAJBIFQYDIOB4IBCEQTCQKRMFYY"
			"DENBWHA5DYPSAIJCEMSCKJRHFAUSUKZMFUXC6MBR"
			"GIZTINJWG44DSOR3HQ6T4P2AIFBEGRCFIZDUQSKK"
			"JNGE2TSPKBIVEU2UKVLFOWCZLJNVYXK6L5QGCYTD"
			"MRSWMZ3INFVGW3DNNZXXA4LSON2HK5TXPB4XU634"
			"PV7H7AEBQKBYJBMGQ6EITCULRSGY5D4QSGJJHFEV"
			"S2LZRGM2TOOJ3HU7UCQ2FI5EUWTKPKFJVKV2ZLNO"
			"V6YLDMVTWS23NN5YXG5LXPF5X274BQOCYPCMLRWH"
			"ZDE4VS6MZXHM7UGR2LJ5JVOW27MNTWW33TO55X7A"
			"4HROHZHF43T6R2PK5PWO33XP6DY7F47U6X3PP6HZ"
			"7L57Z7P674AACAQD";
		EXPECT_EQ(expected, encoded);
	}

	TEST(TEST_CLASS, EncodeCanEncodeInPlace) {
		// Arrange:
		auto input = test::HexStringToVector(Decoded_Set[0]);
		auto output = std::string(Encoded_String_Size, '0');

		// Act:
		Base32Encode(input, output);

		// Assert:
		EXPECT_EQ(Encoded_Set[0], output);
	}

	TEST(TEST_CLASS, EncodeThrowsWhenOutputBufferIsTooSmall) {
		// Arrange:
		auto input = test::HexStringToVector(Decoded_Set[0]);
		auto output = std::string(Encoded_String_Size - 1, '0');

		// Act + Assert:
		EXPECT_THROW(Base32Encode(input, output), catapult_runtime_error);
	}

	TEST(TEST_CLASS, TryEncodeReturnsTrueWhenInputIsValid) {
		// Arrange:
		auto input = test::HexStringToVector(Decoded_Set[0]);
		auto output = std::string(Encoded_String_Size, '0');

		// Act:
		auto result = TryBase32Encode(input, output);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(Encoded_Set[0], output);
	}

	TEST(TEST_CLASS, TryEncodeReturnsFalseWhenInputIsBad) {
		// Arrange:
		std::string illegalStringSet[] {
			"68BA9E8D1AA4502E1F73DA19784B5D7DA16CA1E4AF895FAC12AAAAAAAAAA", // too long
			"68BA9E8D1AA4502E1F73DA19784B5D7DA16CA1E4AF895FAC" // not multiple
		};
		auto output = std::string(Encoded_String_Size, '0');
		for (const auto& input : illegalStringSet) {
			// Act:
			auto inputData = test::HexStringToVector(input);
			auto result = TryBase32Encode(inputData, output);

			// Assert:
			EXPECT_FALSE(result) << input;
		}
	}

	// endregion

	// region Base32Decode

	namespace {
		struct PaddedAddress_tag { static constexpr size_t Size = 25; };
		using PaddedAddress = utils::ByteArray<PaddedAddress_tag>;

		auto HexStringToArray(const std::string& str) {
			return utils::ParseByteArray<PaddedAddress>(str);
		}
	}

	TEST(TEST_CLASS, GetDecodedDataSizeReturnsCorrectSizes) {
		EXPECT_EQ(0u, GetDecodedDataSize(0));
		EXPECT_EQ(5u, GetDecodedDataSize(8));
		EXPECT_EQ(10u, GetDecodedDataSize(16));
		EXPECT_EQ(10u, GetDecodedDataSize(23));
		EXPECT_EQ(15u, GetDecodedDataSize(24));
		EXPECT_EQ(15u, GetDecodedDataSize(25));
	}

	TEST(TEST_CLASS, DecodeEmptyStringReturnsVectorOfSizeZero) {
		// Act:
		auto decoded = Base32Decode(RawString("", 0));

		// Assert:
		EXPECT_TRUE(decoded.empty());
	}

	TEST(TEST_CLASS, DecodeSampleTestVectors) {
		EXPECT_EQ(CountOf(Decoded_Set), CountOf(Encoded_Set));
		for (size_t i = 0; i < CountOf(Encoded_Set); ++i) {
			// Act:
			auto decoded = Base32Decode(RawString(Encoded_Set[i], strlen(Encoded_Set[i])));

			// Assert:
			EXPECT_EQ(test::HexStringToVector(Decoded_Set[i]), decoded);
		}
	}

	TEST(TEST_CLASS, DecodeThrowsWhenStringLengthIsNotAMultipleOfEight) {
		for (auto i = 1u; i < 8; ++i) {
			// Arrange:
			auto str = std::string(i, 'A');

			// Act + Assert:
			EXPECT_THROW(Base32Decode(str), catapult_runtime_error);
		}
	}

	TEST(TEST_CLASS, DecodeAcceptsAllValidCharacters) {
		// Arrange:
		std::string validString{ "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567" };

		// Act:
		auto decoded = Base32Decode(validString);

		// Assert:
		EXPECT_EQ(test::HexStringToVector("00443214C74254B635CF84653A56D7C675BE77DF"), decoded);
	}

	TEST(TEST_CLASS, DecodeThrowsWhenStringContainsIllegalCharacter) {
		// Arrange:
		std::string illegalStringSet[] {
			"NC5J5DI2URIC4H3T3IMXQS21PWQWZIPEV6EV7LAS", // contains char '1'
			"NBGCMBPFWNTLXFF4GB2V5SPV!V2OQD6JFA6SBYUD", // contains char '!'
			"NDL3BGQUX2T44BQOOHAPVGWJWQRG3YLHAE)6CCZ5", // contains char ')'
		};

		for (size_t i = 0; i < CountOf(illegalStringSet); ++i) {
			// Arrange:
			auto str = illegalStringSet[i];

			// Act + Assert:
			EXPECT_THROW(Base32Decode(str), catapult_runtime_error);
		}
	}

	TEST(TEST_CLASS, DecodeCanDecodeInPlace) {
		// Arrange:
		auto input = std::string(Encoded_Set[0]);
		auto output = test::GenerateRandomArray<Decoded_String_Size>();

		// Act:
		Base32Decode(input, output);

		// Assert:
		EXPECT_EQ(HexStringToArray(Decoded_Set[0]), output);
	}

	TEST(TEST_CLASS, DecodeThrowsWhenOutputBufferIsTooSmall) {
		// Arrange:
		auto input = std::string(Encoded_Set[0]);
		auto output = test::GenerateRandomArray<Decoded_String_Size - 1>();

		// Act + Assert:
		EXPECT_THROW(Base32Decode(input, output), catapult_runtime_error);
	}

	TEST(TEST_CLASS, DecodeCanDecodeIntoArray) {
		// Arrange:
		auto input = std::string(Encoded_Set[0]);

		// Act:
		auto output = Base32Decode<Decoded_String_Size>(input);

		// Assert:
		EXPECT_EQ(HexStringToArray(Decoded_Set[0]), output);
	}

	TEST(TEST_CLASS, TryDecodeReturnsTrueWhenInputIsValid) {
		// Arrange:
		auto input = std::string(Encoded_Set[0]);
		auto output = test::GenerateRandomArray<Decoded_String_Size>();

		// Act:
		auto result = TryBase32Decode(input, output);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(HexStringToArray(Decoded_Set[0]), output);
	}

	TEST(TEST_CLASS, TryDecodeReturnsFalseWhenInputIsBad) {
		// Arrange:
		std::string illegalStringSet[] {
			"NC5J5DI2URIC4H3T3IMXQS25PWQWZIPEV6EV1LAS", // contains invalid char
			"NC5J5DI2URIC4H3T3IMXQS25PWQWZIPEV6EV7LASAAAAAAAAAAAAAAAA", // too long
			"NC5J5DI2URIC4H3T3IMXQS25PWQWZIPEV6EV" // not multiple
		};
		auto output = test::GenerateRandomArray<Decoded_String_Size>();
		for (const auto& input : illegalStringSet) {
			// Act:
			auto result = TryBase32Decode(input, output);

			// Assert:
			EXPECT_FALSE(result) << input;
		}
	}

	// endregion

	// region roundtrip

	TEST(TEST_CLASS, RoundtripStartingFromEncodedDoesNotChangeAnything) {
		std::string encodedForRoundtrip[] {
			"BDS73DQ5NC33MKYI3K6GXLJ53C2HJ35A",
			"46FNYP7T4DD3SWAO6C4NX62FJI5CBA26"
		};

		for (size_t i = 0; i < CountOf(encodedForRoundtrip); ++i) {
			// Act:
			auto decoded = Base32Decode(encodedForRoundtrip[i]);
			auto actual = Base32Encode(decoded);

			// Assert:
			EXPECT_EQ(encodedForRoundtrip[i], actual.data());
		}
	}

	TEST(TEST_CLASS, RoundtripStartingFromDecodedDoesNotChangeAnything) {
		// Arrange:
		std::string decodedForRoundtrip[] {
			"8A4E7DF5B61CC0F97ED572A95F6ACA",
			"2D96E4ABB65F0AD3C29FEA48C132CE"
		};

		for (size_t i = 0; i < CountOf(decodedForRoundtrip); ++i) {
			// Act:
			auto buffer = test::HexStringToVector(decodedForRoundtrip[i]);
			auto encoded = Base32Encode(buffer);
			auto actual = Base32Decode(encoded);

			// Assert:
			EXPECT_EQ(test::HexStringToVector(decodedForRoundtrip[i]), actual);
		}
	}

	// endregion
}}
