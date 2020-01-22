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

#include "catapult/crypto/AesCbcDecrypt.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/crypto/EncryptionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS AesCbcDecryptTests

	// region TryAesCbcDecrypt

	namespace {
		SharedKey ParseSharedKey(const std::string& sharedKeyStr) {
			return utils::ParseByteArray<SharedKey>(sharedKeyStr);
		}

		auto Concatenate(const std::string& initializationVectorStr, const std::string& cipherText) {
			auto initializationVector = test::HexStringToVector(initializationVectorStr);
			auto encrypted = test::HexStringToVector(cipherText);

			initializationVector.insert(initializationVector.end(), encrypted.cbegin(), encrypted.cend());
			return initializationVector;
		}

		void RunValidTestCase(
				const std::string& keyStr,
				const std::string& initializationVectorStr,
				const std::string& cipherText,
				const std::string& clearText) {
			// Arrange:
			auto key = ParseSharedKey(keyStr);
			auto encrypted = Concatenate(initializationVectorStr, cipherText);
			auto expected = test::HexStringToVector(clearText);

			// Act:
			std::vector<uint8_t> decrypted;
			auto decryptionResult = TryAesCbcDecrypt(key, encrypted, decrypted);

			// Assert:
			EXPECT_TRUE(decryptionResult);
			EXPECT_EQ(expected, decrypted);
		}

		void RunInvalidTestCase(const std::string& cipherText) {
			// Arrange: all invalid tests share same key and iv data
			auto key = ParseSharedKey("7C78F34DBCE8F0557D43630266F59BABD1CB92BA624BD1A8F45A2A91C84A804A");
			auto encrypted = Concatenate("F010F61C31C9AA8FA0D5BE5F6B0F2F70", cipherText);

			// Act:
			std::vector<uint8_t> decrypted;
			auto decryptionResult = TryAesCbcDecrypt(key, encrypted, decrypted);

			// Assert:
			EXPECT_FALSE(decryptionResult);
		}

		KeyPair GenerateKeyPair() {
			return KeyPair::FromPrivate(PrivateKey::Generate(test::RandomByte));
		}
	}

	// test vectors taken from wycheproof project:
	// https://github.com/google/wycheproof/blob/master/testvectors/aes_cbc_pkcs5_test.json

	TEST(TEST_CLASS, CanDecryptEmptyMessage) {
		// Assert: tcId: 123
		RunValidTestCase(
				"7BF9E536B66A215C22233FE2DAAA743A898B9ACB9F7802DE70B40E3D6E43EF97",
				"EB38EF61717E1324AE064E86F1C3E797",
				"E7C166554D1BB32792C981FA674CC4D8",
				"");
	}

	TEST(TEST_CLASS, CanDecryptMessageDivisibleByBlockSize) {
		// Assert: tcId: 124
		RunValidTestCase(
				"612E837843CEAE7F61D49625FAA7E7494F9253E20CB3ADCEA686512B043936CD",
				"9EC7B863AC845CAD5E4673DA21F5B6A9",
				"299295BE47E9F5441FE83A7A811C4AEB2650333E681E69FA6B767D28A6CCF282",
				"CC37FAE15F745A2F40E2C8B192F2B38D");
	}

	TEST(TEST_CLASS, CanDecryptMessageWithShortPlaintext) {
		// Assert: tcId: 127
		RunValidTestCase(
				"E754076CEAB3FDAF4F9BCAB7D4F0DF0CBBAFBC87731B8F9B7CD2166472E8EEBC",
				"014D2E13DFBCB969BA3BB91442D52ECA",
				"42C0B89A706ED2606CD94F9CB361FA51",
				"40");
	}

	TEST(TEST_CLASS, CanDecryptMessageWithPlaintextLargerThanBlockSize) {
		// Assert: tcId: 142
		RunValidTestCase(
				"73216FAFD0022D0D6EE27198B2272578FA8F04DD9F44467FBB6437AA45641BF7",
				"4B74BD981EA9D074757C3E2EF515E5FB",
				"FBEA776FB1653635F88E2937ED2450BA4E9063E96D7CDBA04928F01CB85492FE",
				"D5247B8F6C3EDCBFB1D591D13ECE23D2F5");
	}

	TEST(TEST_CLASS, CannotDecryptMessageWithZeroPadding) {
		// Assert: tcId: 147
		// resulting cleartext prior to dropping padding is block of sixteen 0-bytes
		RunInvalidTestCase("E07558D746574528FB813F34E3FB7719");
	}

	TEST(TEST_CLASS, CannotDecryptMessageWithBitPadding_EndsWithZero) {
		// Assert: tcId: 158
		// due to bit padding used, last pad byte == 0
		RunInvalidTestCase("94D18B5923F8F3608AE7AD494FBB517E");
	}

	TEST(TEST_CLASS, CannotDecryptMessageWithBitPadding_EndsWithBitMarker) {
		// Assert: tcId: 159
		// due to bit padding used, last byte == 0x80
		RunInvalidTestCase("0C92886DBCB030B873123A25D224DA42");
	}

	TEST(TEST_CLASS, CannotDecryptMessageWithPaddingLargerThanBlockSize) {
		// Assert: tcId: 162
		RunInvalidTestCase("524236E25956E950713BEC0D3D579068F34E4D18C4CCAB081317DAE526FE7FCA");
	}

	TEST(TEST_CLASS, CannotDecryptMessageWithAnsiX923Padding) {
		// Assert: tcId: 168
		// padding is \0\0\0\0\0\0\0\8 - that should fail in std::any()
		RunInvalidTestCase("18CF8988ABE9A2463A3A75DB1FAC8BCC");
	}

	TEST(TEST_CLASS, CannotDecryptMessageWithIso10126Padding) {
		// Assert: tcId: 172
		// padding with random bytes
		RunInvalidTestCase("C477877250C8E4CA2869F35C4757CDB4");
	}

	TEST(TEST_CLASS, CannotDecryptMessageWithInvalidPadding_BorderCase1) {
		// Assert: tcId: 180
		// border case, padding is proper except for first padding byte (\0)
		RunInvalidTestCase("32AC6057DF2A5D1E2E5131348C6EBC4E");
	}

	TEST(TEST_CLASS, CannotDecryptMessageWithInvalidPadding_BorderCase2) {
		// Assert: tcId: 181
		// border case, padding is proper except for first padding byte (\0)
		RunInvalidTestCase("DF4A7C3B9F4756D30FCA0D18E9B28960");
	}

	TEST(TEST_CLASS, CannotDecryptMessageWithMisalignedSize) {
		// Assert: generate valid data, but drop random amount of bytes from result
		auto sharedKey = test::GenerateRandomByteArray<SharedKey>();
		auto initializationVector = test::GenerateRandomByteArray<AesInitializationVector>();
		auto input = test::GenerateRandomVector(16);
		std::vector<uint8_t> encrypted;
		test::AesCbcEncrypt(sharedKey, initializationVector, input, encrypted, test::AesPkcs7PaddingScheme);

		// - drop random amount between 1-15
		auto dropSize = 1u + test::RandomByte() % 15;
		encrypted.resize(encrypted.size() - dropSize);

		// Act:
		std::vector<uint8_t> decrypted;
		auto result = TryAesCbcDecrypt(sharedKey, encrypted, decrypted);

		// Assert:
		EXPECT_FALSE(result) << "drop size: " << dropSize;
	}

	// endregion

	// region TryDecryptEd25199BlockCipher

	TEST(TEST_CLASS, Ed25199BlockCipher_ThrowsWhenEncryptedDataIsTooSmall) {
		// Arrange:
		auto keyPair = GenerateKeyPair();
		auto encryptedWithKey = test::GenerateRandomVector(Key::Size - 1);

		// Act + Assert:
		std::vector<uint8_t> decrypted;
		EXPECT_THROW(TryDecryptEd25199BlockCipher(encryptedWithKey, keyPair, decrypted), catapult_invalid_argument);
	}

	namespace {
		void AssertNotEnoughDataFailure(size_t size) {
			// Arrange:
			auto keyPair = GenerateKeyPair();
			auto encryptedWithKey = test::GenerateRandomVector(size);

			// Act:
			std::vector<uint8_t> decrypted;
			auto result = TryDecryptEd25199BlockCipher(encryptedWithKey, keyPair, decrypted);

			// Assert:
			EXPECT_FALSE(result);
		}
	}

	TEST(TEST_CLASS, Ed25199BlockCipher_FailsWhenEncryptedDataDoesNotContainInitializationVector) {
		AssertNotEnoughDataFailure(Key::Size + AesInitializationVector::Size - 1);
	}

	TEST(TEST_CLASS, Ed25199BlockCipher_FailsWhenEncryptedDataDoesNotContainPadding) {
		AssertNotEnoughDataFailure(Key::Size + AesInitializationVector::Size + 16 - 1);
	}

	namespace {
		void AssertDecryptEd25199BlockCipher(size_t dataSize, size_t expectedEncryptedSize) {
			// Arrange:
			auto clearText = test::GenerateRandomVector(dataSize);
			auto recipientKeyPair = GenerateKeyPair();
			auto prefixedEncrypted = test::GenerateEphemeralAndEncrypt(clearText, recipientKeyPair.publicKey());

			// Sanity:
			EXPECT_EQ(expectedEncryptedSize, prefixedEncrypted.size());

			// Act:
			std::vector<uint8_t> decrypted;
			auto result = TryDecryptEd25199BlockCipher(prefixedEncrypted, recipientKeyPair, decrypted);

			// Assert:
			EXPECT_TRUE(result);
			EXPECT_EQ(clearText, decrypted);
		}
	}

	TEST(TEST_CLASS, Ed25199BlockCipher_CanDecryptEmptyMessage) {
		AssertDecryptEd25199BlockCipher(0, Key::Size + AesInitializationVector::Size + 16);
	}

	TEST(TEST_CLASS, Ed25199BlockCipher_CanDecryptMessage) {
		// padding size = 16 - (123 % 16) = 5
		AssertDecryptEd25199BlockCipher(123, Key::Size + AesInitializationVector::Size + 123 + 5);
	}

	// endregion
}}
