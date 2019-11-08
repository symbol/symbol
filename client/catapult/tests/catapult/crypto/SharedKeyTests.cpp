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
#include "catapult/crypto/KeyUtils.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS SharedKeyTests

	TEST(TEST_CLASS, PassesTestVector) {
		// Arrange: private key used is the one from KeyPairTests
#ifdef SIGNATURE_SCHEME_KECCAK
		auto expectedSharedKey = utils::ParseByteArray<SharedKey>("E9BF812E9E29B1D4C8D01E3DA11EAB3715A582CD2AA66EABBDAFEA7DFB9B2422");
#else
		auto expectedSharedKey = utils::ParseByteArray<SharedKey>("3B3524D2E92F89423456E43A3FD25C52C71CA4C680C32F022C23506BB23BDB0C");
#endif

		auto rawKeyString = std::string("ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57");
		auto keyPair = KeyPair::FromString(rawKeyString);
		auto otherPublicKey = ParseKey("0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF");
		auto salt = utils::ParseByteArray<Salt>("C06B2CC5D7B66900B2493CF68BE10B7AA8690D973B7F0B65D0DAE4F7AA464716");

		// Sanity: otherPublicKey is actually some *other* account
		EXPECT_NE(keyPair.publicKey(), otherPublicKey);

		// Act:
		auto sharedKey = DeriveSharedKey(keyPair, otherPublicKey, salt);

		// Assert:
		EXPECT_EQ(expectedSharedKey, sharedKey);
	}

	namespace {
		auto CreateKeyPair(const Key& privateKey) {
			auto index = 0u;
			return KeyPair::FromPrivate(PrivateKey::Generate([&privateKey, &index]() { return privateKey[index++]; }));
		}

		void AssertDeriveSharedKey(
				const consumer<Key&, Key&, Salt&>& mutate,
				const consumer<const SharedKey&, const SharedKey&>& assertKeys) {
			// Arrange: the public key needs to be valid, else unpacking will fail
			auto privateKey1 = test::GenerateRandomByteArray<Key>();
			auto otherPublicKey1 = test::GenerateKeyPair().publicKey();
			auto salt1 = test::GenerateRandomByteArray<Salt>();

			auto privateKey2 = privateKey1;
			auto otherPublicKey2 = otherPublicKey1;
			auto salt2 = salt1;

			mutate(privateKey2, otherPublicKey2, salt2);

			auto keyPair1 = CreateKeyPair(privateKey1);
			auto keyPair2 = CreateKeyPair(privateKey2);

			// Act:
			auto sharedKey1 = DeriveSharedKey(keyPair1, otherPublicKey1, salt1);
			auto sharedKey2 = DeriveSharedKey(keyPair2, otherPublicKey2, salt2);

			// Assert:
			assertKeys(sharedKey1, sharedKey2);
		}

		void AssertDerivedSharedKeysAreEqual(const consumer<Key&, Key&, Salt&>& mutate) {
			AssertDeriveSharedKey(mutate, [](const auto& sharedKey1, const auto& sharedKey2) {
				EXPECT_EQ(sharedKey1, sharedKey2);
			});
		}

		void AssertDerivedSharedKeysAreDifferent(const consumer<Key&, Key&, Salt&>& mutate) {
			AssertDeriveSharedKey(mutate, [](const auto& sharedKey1, const auto& sharedKey2) {
				EXPECT_NE(sharedKey1, sharedKey2);
			});
		}
	}

	TEST(TEST_CLASS, SharedKeysGeneratedWithSameInputsAreEqual) {
		AssertDerivedSharedKeysAreEqual([] (const auto&, const auto&, const auto&) {});
	}

	TEST(TEST_CLASS, SharedKeysGeneratedForDifferentKeyPairsAreDifferent) {
		AssertDerivedSharedKeysAreDifferent([] (auto& privateKey, const auto&, const auto&) {
			privateKey[0] ^= 0xFF;
		});
	}

	TEST(TEST_CLASS, SharedKeysGeneratedForDifferentOtherPublicKeysAreDifferent) {
		AssertDerivedSharedKeysAreDifferent([] (const auto&, auto& otherPublicKey, const auto&) {
			otherPublicKey[0] ^= 0xFF;
		});
	}

	TEST(TEST_CLASS, SharedKeysGeneratedWithDifferentSaltsAreDifferent) {
		AssertDerivedSharedKeysAreDifferent([] (const auto&, const auto&, auto& salt) {
			salt[0] ^= 0xFF;
		});
	}

	TEST(TEST_CLASS, MutualSharedKeysAreEqual) {
		// Arrange:
		auto privateKey1 = test::GenerateRandomByteArray<Key>();
		auto privateKey2 = test::GenerateRandomByteArray<Key>();
		auto keyPair1 = CreateKeyPair(privateKey1);
		auto keyPair2 = CreateKeyPair(privateKey2);

		auto salt = test::GenerateRandomByteArray<Salt>();

		// Act:
		auto sharedKey1 = DeriveSharedKey(keyPair1, keyPair2.publicKey(), salt);
		auto sharedKey2 = DeriveSharedKey(keyPair2, keyPair1.publicKey(), salt);

		// Assert:
		EXPECT_EQ(sharedKey2, sharedKey1);
	}

	TEST(TEST_CLASS, PublicKeyNotOnTheCurveResultsInZeroSharedKey) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		Key publicKey{};
		publicKey[Key::Size - 1] = 1; // not on the curve

		auto salt = test::GenerateRandomByteArray<Salt>();

		// Act:
		auto sharedKey = DeriveSharedKey(keyPair, publicKey, salt);

		// Assert:
		EXPECT_EQ(SharedKey(), sharedKey);
	}
}}
