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

#include "catapult/crypto/KeyPair.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS KeyPairTests

	namespace {
		void AssertCannotCreatePrivateKeyFromStringWithSize(size_t size, char keyFirstChar) {
			// Arrange:
			auto rawKeyString = test::GenerateRandomHexString(size);
			rawKeyString[0] = keyFirstChar;

			// Act + Assert: key creation should fail but string should not be cleared
			EXPECT_THROW(KeyPair::FromString(rawKeyString), catapult_invalid_argument) << "string size: " << size;
			EXPECT_EQ(keyFirstChar, rawKeyString[0]);
		}
	}

	TEST(TEST_CLASS, CannotCreateKeyPairFromInvalidString) {
		AssertCannotCreatePrivateKeyFromStringWithSize(Key::Size * 1, 'a');
		AssertCannotCreatePrivateKeyFromStringWithSize(Key::Size * 2, 'g');
		AssertCannotCreatePrivateKeyFromStringWithSize(Key::Size * 3, 'a');
	}

	TEST(TEST_CLASS, CanCreateKeyPairFromValidString) {
		// Arrange:
		auto rawKeyString = std::string("CBD84EF8F5F38A25C01308785EA99627DE897D151AFDFCDA7AB07EFD8ED98534");
		auto expectedKey = utils::ParseByteArray<Key>("E343795087E44BC0CD516F3FF19954A9B90FEA7684724E5C145559D6D4D9F56D");
		auto rawKeyVector = test::HexStringToVector(rawKeyString);

		// Act:
		auto keyPair = KeyPair::FromString(rawKeyString);

		// Assert:
		EXPECT_EQ(expectedKey, keyPair.publicKey());
		EXPECT_EQ_MEMORY(&rawKeyVector[0], keyPair.privateKey().data(), keyPair.privateKey().size());
	}

	TEST(TEST_CLASS, CanCreateKeyPairFromPrivateKey) {
		// Arrange:
		auto privateKeyStr = test::GenerateRandomHexString(Key::Size * 2);
		auto privateKey = PrivateKey::FromString(privateKeyStr);
		Key expectedPublicKey;
		ExtractPublicKeyFromPrivateKey(privateKey, expectedPublicKey);

		// Act:
		auto keyPair = KeyPair::FromPrivate(std::move(privateKey));

		// Assert:
		EXPECT_EQ(PrivateKey::FromString(privateKeyStr), keyPair.privateKey());
		EXPECT_EQ(expectedPublicKey, keyPair.publicKey());
	}

	TEST(TEST_CLASS, KeyPairCreatedFromPrivateKeyMatchesKeyPairCreatedFromString) {
		// Arrange:
		auto privateKeyStr = std::string("3485D98EFD7EB07ADAFCFD1A157D89DE2796A95E780813C0258AF3F5F84ED8CB");
		auto privateKey = PrivateKey::FromString(privateKeyStr);

		// Act:
		auto keyPair1 = KeyPair::FromPrivate(std::move(privateKey));
		auto keyPair2 = KeyPair::FromString(privateKeyStr);

		// Assert:
		EXPECT_EQ(keyPair1.privateKey(), keyPair2.privateKey());
	}

	TEST(TEST_CLASS, PassesNemTestVectors) {
		// Arrange:
		// from nem https://github.com/nemtech/test-vectors
		std::string dataSet[] {
			"ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57",
			"FE9BC2EF8DF88E708CAB471F82B54DBFCBA11B121E7C2D02799AB4D3A53F0E5B",
			"DAEE5A32E12CEDEFD0349FDBA1FCBDB45356CA3A35AA5CF1A8AE1091BBA98B73",
			"A6954BAA315EE50453CCE3483906F134405B8B3ADD94BFC8D8125CF3C09BBFE8",
			"832A237053A83B7E97CA287AC15F9AD5838898E7395967B56D39749652EA25C3"
		};
		std::string expectedSet[] {
			"5112BA143B78132AF616AF1A94E911EAD890FDB51B164A1B57C352ECD9CA1894",
			"5F9EB725880D0B8AC122AD2939070172C8762713A1E29CE55EEEA0BFBA05E6DB",
			"2D8C6B2B1D69CC02464339F46A788D7A5A6D7875C9D12AAD4ACCF2D5B24887FC",
			"20E7F2BC716306F70A136121DC103604FD624328BCEA81E5786F3CB4CE96E60E",
			"2470623117B439AA09C487D0F3D4B23676565DC1478F7C7443579B0255FE6DE1"
		};

		ASSERT_EQ(CountOf(dataSet), CountOf(expectedSet));
		for (size_t i = 0; i < CountOf(dataSet); ++i) {
			// Act:
			auto keyPair = KeyPair::FromString(dataSet[i]);

			// Assert:
			EXPECT_EQ(utils::ParseByteArray<Key>(expectedSet[i]), keyPair.publicKey());
		}
	}
}}
