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

#include "catapult/crypto/AesDecrypt.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/crypto/EncryptionTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS AesDecryptTests

	// region AesGcm256::TryDecrypt

	namespace {
		SharedKey ParseSharedKey(const std::string& sharedKeyStr) {
			return utils::ParseByteArray<SharedKey>(sharedKeyStr);
		}

		auto Concatenate(const std::string& tagStr, const std::string& ivStr, const std::string& cipherText) {
			return test::HexStringToVector(tagStr + ivStr + cipherText);
		}

		void RunValidTestCase(
				const std::string& keyStr,
				const std::string& ivStr,
				const std::string& tagStr,
				const std::string& cipherText,
				const std::string& clearText) {
			// Arrange:
			auto key = ParseSharedKey(keyStr);
			auto encrypted = Concatenate(tagStr, ivStr, cipherText);
			auto expected = test::HexStringToVector(clearText);

			// Act:
			std::vector<uint8_t> decrypted;
			auto decryptionResult = AesGcm256::TryDecrypt(key, encrypted, decrypted);

			// Assert:
			EXPECT_TRUE(decryptionResult);
			EXPECT_EQ(expected, decrypted);
		}

		void RunInvalidTestCase(const std::string& tagStr) {
			// Arrange:
			auto key = ParseSharedKey("000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F");
			auto encrypted = Concatenate(tagStr, "505152535455565758595A5B", "B2061457C0759FC1749F174EE1CCADFA");

			// Act:
			std::vector<uint8_t> decrypted;
			auto decryptionResult = AesGcm256::TryDecrypt(key, encrypted, decrypted);

			// Assert:
			EXPECT_FALSE(decryptionResult);
		}
	}

	// test vectors taken from wycheproof project:
	// https://github.com/google/wycheproof/blob/master/testvectors/aes_gcm_test.json

	TEST(TEST_CLASS, CanDecryptEmptyMessage) {
		// Assert: tcId: 75
		RunValidTestCase(
				"80BA3192C803CE965EA371D5FF073CF0F43B6A2AB576B208426E11409C09B9B0",
				"4DA5BF8DFD5852C1EA12379D",
				"4771A7C404A472966CEA8F73C8BFE17A",
				"",
				"");
	}

	TEST(TEST_CLASS, CanDecryptMessageWithShortPlaintext) {
		// Assert: tcId: 76
		RunValidTestCase(
				"CC56B680552EB75008F5484B4CB803FA5063EBD6EAB91F6AB6AEF4916A766273",
				"99E23EC48985BCCDEEAB60F1",
				"633C1E9703EF744FFFFB40EDF9D14355",
				"06",
				"2A");
	}

	TEST(TEST_CLASS, CanDecryptMessageWithLongPlaintext) {
		// Assert: tcId: 87
		RunValidTestCase(
				"D7ADDD3889FADF8C893EEE14BA2B7EA5BF56B449904869615BD05D5F114CF377",
				"8A3AD26B28CD13BA6504E260",
				"5E63374B519E6C3608321943D790CF9A",
				"53CC8C920A85D1ACCB88636D08BBE4869BFDD96F437B2EC944512173A9C0FE7A"
				"47F8434133989BA77DDA561B7E3701B9A83C3BA7660C666BA59FEF96598EB621"
				"544C63806D509AC47697412F9564EB0A2E1F72F6599F5666AF34CFFCA06573FF"
				"B4F47B02F59F21C64363DAECB977B4415F19FDDA3C9AAE5066A57B669FFAA257",
				"C877A76BF595560772167C6E3BCC705305DB9C6FCBEB90F4FEA85116038BC53C"
				"3FA5B4B4EA0DE5CC534FBE1CF9AE44824C6C2C0A5C885BD8C3CDC906F1267573"
				"7E434B983E1E231A52A275DB5FB1A0CAC6A07B3B7DCB19482A5D3B06A9317A54"
				"826CEA6B36FCE452FA9B5475E2AAF25499499D8A8932A19EB987C903BD8502FE");
	}

	TEST(TEST_CLASS, CannotDecryptMessageWithFlippedTagBits) {
		// Assert: tcId: 114
		RunInvalidTestCase("9CE8FE76D8AB1B71BF887232EAB590DD");
	}

	TEST(TEST_CLASS, CannotDecryptMessageWithMsbsChangedInTag) {
		// Assert: tcId: 119
		RunInvalidTestCase("1C687E76582B9B713F08F2B26A35105D");
	}

	TEST(TEST_CLASS, CannotDecryptMessageWithLsbsChangedInTag) {
		// Assert: tcId: 120
		RunInvalidTestCase("9DE9FFF7D9AA1AF0BE897333EBB491DC");
	}

	// endregion

	// region TryDecryptEd25199BlockCipher

	TEST(TEST_CLASS, Ed25199BlockCipher_ThrowsWhenEncryptedDataIsTooSmall) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		auto publicKeyPrefixedEncryptedPayload = test::GenerateRandomVector(Key::Size - 1);

		// Act + Assert:
		std::vector<uint8_t> decrypted;
		EXPECT_THROW(TryDecryptEd25199BlockCipher(publicKeyPrefixedEncryptedPayload, keyPair, decrypted), catapult_invalid_argument);
	}

	namespace {
		void AssertNotEnoughDataFailure(size_t size) {
			// Arrange:
			auto keyPair = test::GenerateKeyPair();
			auto publicKeyPrefixedEncryptedPayload = test::GenerateRandomVector(size);

			// Act:
			std::vector<uint8_t> decrypted;
			auto result = TryDecryptEd25199BlockCipher(publicKeyPrefixedEncryptedPayload, keyPair, decrypted);

			// Assert:
			EXPECT_FALSE(result);
		}
	}

	TEST(TEST_CLASS, Ed25199BlockCipher_FailsWhenEncryptedDataDoesNotContainTag) {
		AssertNotEnoughDataFailure(Key::Size + AesGcm256::Tag::Size - 1);
	}

	TEST(TEST_CLASS, Ed25199BlockCipher_FailsWhenEncryptedDataDoesNotContainInitializationVector) {
		AssertNotEnoughDataFailure(Key::Size + AesGcm256::Tag::Size + AesGcm256::IV::Size - 1);
	}

	namespace {
		void AssertDecryptEd25199BlockCipher(size_t dataSize, size_t expectedEncryptedSize) {
			// Arrange:
			auto clearText = test::GenerateRandomVector(dataSize);
			auto recipientKeyPair = test::GenerateKeyPair();
			auto publicKeyPrefixedEncryptedPayload = test::GenerateEphemeralAndEncrypt(clearText, recipientKeyPair.publicKey());

			// Sanity:
			EXPECT_EQ(expectedEncryptedSize, publicKeyPrefixedEncryptedPayload.size());

			// Act:
			std::vector<uint8_t> decrypted;
			auto result = TryDecryptEd25199BlockCipher(publicKeyPrefixedEncryptedPayload, recipientKeyPair, decrypted);

			// Assert:
			EXPECT_TRUE(result);
			EXPECT_EQ(clearText, decrypted);
		}
	}

	TEST(TEST_CLASS, Ed25199BlockCipher_CanDecryptEmptyMessage) {
		AssertDecryptEd25199BlockCipher(0, Key::Size + AesGcm256::Tag::Size + AesGcm256::IV::Size);
	}

	TEST(TEST_CLASS, Ed25199BlockCipher_CanDecryptMessage) {
		AssertDecryptEd25199BlockCipher(123, Key::Size + AesGcm256::Tag::Size + AesGcm256::IV::Size + 123);
	}

	// endregion
}}
