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

#include "catapult/crypto/SharedKey.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS SharedKeyTests

	// region HKDF Sha256
	// data taken from : RFC 5869

	namespace {
		// generic implementation to check test vectors
		void Hkdf_Hmac_Sha256(const RawBuffer& sharedSecret, const RawBuffer& salt, const RawBuffer& label, std::vector<uint8_t>& output) {
			Hash256 prk;
			Hmac_Sha256(salt, sharedSecret, prk);

			// T(i - 1) || label || counter
			std::vector<uint8_t> buffer;
			buffer.resize(Hash256::Size + label.Size + sizeof(uint8_t));

			size_t repetitions = (output.size() + Hash256::Size - 1) / Hash256::Size;
			size_t position = 0;

			// inline first iteration
			uint32_t counter = 1;
			std::memcpy(buffer.data(), label.pData, label.Size);
			std::memcpy(buffer.data() + label.Size, &counter, sizeof(uint8_t));

			Hash256 previousOkm;
			Hmac_Sha256(prk, { buffer.data(), label.Size + sizeof(uint8_t) }, previousOkm);

			auto numBytesWritten = std::min(output.size() - position, Hash256::Size);
			std::memcpy(&output[position], previousOkm.data(), numBytesWritten);
			position += numBytesWritten;
			++counter;

			for (; counter <= repetitions; ++counter) {
				std::memcpy(buffer.data(), previousOkm.data(), Hash256::Size);
				std::memcpy(buffer.data() + Hash256::Size, label.pData, label.Size);
				std::memcpy(buffer.data() + Hash256::Size + label.Size, &counter, sizeof(uint8_t));

				Hmac_Sha256(prk, buffer, previousOkm);

				numBytesWritten = std::min(output.size() - position, Hash256::Size);
				std::memcpy(&output[position], previousOkm.data(), numBytesWritten);
				position += numBytesWritten;
			}
		}
	}

	TEST(TEST_CLASS, Hkdf_Hmac_Sha256_Test_Vector_1) {
		// Arrange:
		auto salt = test::HexStringToVector("000102030405060708090A0B0C");
		auto sharedSecret = test::HexStringToVector("0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B");
		auto label = test::HexStringToVector("F0F1F2F3F4F5F6F7F8F9");
		auto expected = test::HexStringToVector("3CB25F25FAACD57A90434F64D0362F2A2D2D0A90CF1A5A4C5DB02D56ECC4C5BF34007208D5B887185865");

		// Act:
		std::vector<uint8_t> output(expected.size());
		Hkdf_Hmac_Sha256(sharedSecret, salt, label, output);

		// Assert:
		EXPECT_EQ(expected, output);
	}

	TEST(TEST_CLASS, Hkdf_Hmac_Sha256_Test_Vector_2) {
		// Arrange:
		auto salt = test::HexStringToVector(
				"606162636465666768696A6B6C6D6E6F707172737475767778797A7B7C7D7E7F"
				"808182838485868788898A8B8C8D8E8F909192939495969798999A9B9C9D9E9F"
				"A0A1A2A3A4A5A6A7A8A9AAABACADAEAF");
		auto sharedSecret = test::HexStringToVector(
				"000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F"
				"202122232425262728292A2B2C2D2E2F303132333435363738393A3B3C3D3E3F"
				"404142434445464748494A4B4C4D4E4F");
		auto label = test::HexStringToVector(
				"B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBFC0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
				"D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDFE0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
				"F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF");
		auto expected = test::HexStringToVector(
				"B11E398DC80327A1C8E7F78C596A49344F012EDA2D4EFAD8A050CC4C19AFA97C"
				"59045A99CAC7827271CB41C65E590E09DA3275600C2F09B8367793A9ACA3DB71"
				"CC30C58179EC3E87C14C01D5C1F3434F1D87");

		// Act:
		std::vector<uint8_t> output(expected.size());
		Hkdf_Hmac_Sha256(sharedSecret, salt, label, output);

		// Assert:
		EXPECT_EQ(expected, output);
	}

	TEST(TEST_CLASS, Hkdf_Hmac_Sha256_Test_Vector_3) {
		// Arrange:
		auto salt = test::HexStringToVector("");
		auto sharedSecret = test::HexStringToVector("0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B");
		auto label = test::HexStringToVector("");
		auto expected = test::HexStringToVector("8DA4E775A563C18F715F802A063C5A31B8A11F5C5EE1879EC3454E5F3C738D2D9D201395FAA4B61A96C8");

		// Act:
		std::vector<uint8_t> output(expected.size());
		Hkdf_Hmac_Sha256(sharedSecret, salt, label, output);

		// Assert:
		EXPECT_EQ(expected, output);
	}

	TEST(TEST_CLASS, Hkdf_Hmac_Sha256_32_Tests) {
		// Arrange:
		std::vector<uint8_t> salt(32);
		auto label = test::HexStringToVector("6361746170756c74");

		for (auto i = 0; i < 10; ++i) {
			// - calculate expected output using generic implementation
			auto sharedSecret = test::GenerateRandomByteArray<SharedSecret>();
			std::vector<uint8_t> expectedOutput(32);
			Hkdf_Hmac_Sha256(sharedSecret, salt, label, expectedOutput);

			// Act:
			auto sharedKey = Hkdf_Hmac_Sha256_32(sharedSecret);

			// Assert:
			std::vector<uint8_t> sharedKeyBuffer(sharedKey.cbegin(), sharedKey.cend());
			EXPECT_EQ(expectedOutput, sharedKeyBuffer);
		}
	}

	// endregion

	namespace {
		auto CreateKeyPair(const Key& privateKey) {
			auto index = 0u;
			return KeyPair::FromPrivate(PrivateKey::Generate([&privateKey, &index]() { return privateKey[index++]; }));
		}

		void AssertDerivedSharedKey(const consumer<Key&, Key&>& mutate, const consumer<const SharedKey&, const SharedKey&>& assertKeys) {
			// Arrange: the public key needs to be valid, else unpacking will fail
			auto privateKey1 = test::GenerateRandomByteArray<Key>();
			auto otherPublicKey1 = test::GenerateKeyPair().publicKey();

			auto privateKey2 = privateKey1;
			auto otherPublicKey2 = otherPublicKey1;

			mutate(privateKey2, otherPublicKey2);

			auto keyPair1 = CreateKeyPair(privateKey1);
			auto keyPair2 = CreateKeyPair(privateKey2);

			// Act:
			auto sharedKey1 = DeriveSharedKey(keyPair1, otherPublicKey1);
			auto sharedKey2 = DeriveSharedKey(keyPair2, otherPublicKey2);

			// Assert:
			assertKeys(sharedKey1, sharedKey2);
		}

		void AssertDerivedSharedKeysAreEqual(const consumer<Key&, Key&>& mutate) {
			AssertDerivedSharedKey(mutate, [](const auto& sharedKey1, const auto& sharedKey2) {
				EXPECT_EQ(sharedKey1, sharedKey2);
			});
		}

		void AssertDerivedSharedKeysAreDifferent(const consumer<Key&, Key&>& mutate) {
			AssertDerivedSharedKey(mutate, [](const auto& sharedKey1, const auto& sharedKey2) {
				EXPECT_NE(sharedKey1, sharedKey2);
			});
		}
	}

	TEST(TEST_CLASS, SharedKeysGeneratedWithSameInputsAreEqual) {
		AssertDerivedSharedKeysAreEqual([] (const auto&, const auto&) {});
	}

	TEST(TEST_CLASS, SharedKeysGeneratedForDifferentKeyPairsAreDifferent) {
		AssertDerivedSharedKeysAreDifferent([] (auto& privateKey, const auto&) {
			privateKey[0] ^= 0xFF;
		});
	}

	TEST(TEST_CLASS, SharedKeysGeneratedForDifferentOtherPublicKeysAreDifferent) {
		AssertDerivedSharedKeysAreDifferent([] (const auto&, auto& otherPublicKey) {
			otherPublicKey[0] ^= 0xFF;
		});
	}

	TEST(TEST_CLASS, MutualSharedKeysAreEqual) {
		// Arrange:
		auto privateKey1 = test::GenerateRandomByteArray<Key>();
		auto privateKey2 = test::GenerateRandomByteArray<Key>();
		auto keyPair1 = CreateKeyPair(privateKey1);
		auto keyPair2 = CreateKeyPair(privateKey2);

		// Act:
		auto sharedKey1 = DeriveSharedKey(keyPair1, keyPair2.publicKey());
		auto sharedKey2 = DeriveSharedKey(keyPair2, keyPair1.publicKey());

		// Assert:
		EXPECT_EQ(sharedKey2, sharedKey1);
	}

	TEST(TEST_CLASS, PublicKeyNotOnTheCurveResultsInZeroSharedKey) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		Key publicKey{};
		publicKey[Key::Size - 1] = 1; // not on the curve

		// Act:
		auto sharedKey = DeriveSharedKey(keyPair, publicKey);

		// Assert:
		EXPECT_EQ(SharedKey(), sharedKey);
	}
}}
