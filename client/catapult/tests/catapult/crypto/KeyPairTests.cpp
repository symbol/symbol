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
#ifdef SIGNATURE_SCHEME_KECCAK
		auto rawKeyString = std::string("CBD84EF8F5F38A25C01308785EA99627DE897D151AFDFCDA7AB07EFD8ED98534");
		auto expectedKey = utils::ParseByteArray<Key>("C54D6E33ED1446EEDD7F7A80A588DD01857F723687A09200C1917D5524752F8B");
#else
		auto rawKeyString = std::string("CBD84EF8F5F38A25C01308785EA99627DE897D151AFDFCDA7AB07EFD8ED98534");
		auto expectedKey = utils::ParseByteArray<Key>("A6DC1C33C26BC67B21AC4B3F4D1E88901E23AD208260F40AF3CB0A6CE9557852");
#endif
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
#ifdef SIGNATURE_SCHEME_KECCAK
		// from nem https://github.com/NewEconomyMovement/nem-test-vectors)
		std::string dataSet[] {
			"ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57",
			"FE9BC2EF8DF88E708CAB471F82B54DBFCBA11B121E7C2D02799AB4D3A53F0E5B",
			"DAEE5A32E12CEDEFD0349FDBA1FCBDB45356CA3A35AA5CF1A8AE1091BBA98B73",
			"A6954BAA315EE50453CCE3483906F134405B8B3ADD94BFC8D8125CF3C09BBFE8",
			"832A237053A83B7E97CA287AC15F9AD5838898E7395967B56D39749652EA25C3"
		};
		std::string expectedSet[] {
			"C5F54BA980FCBB657DBAAA42700539B207873E134D2375EFEAB5F1AB52F87844",
			"96EB2A145211B1B7AB5F0D4B14F8ABC8D695C7AEE31A3CFC2D4881313C68EEA3",
			"2D8425E4CA2D8926346C7A7CA39826ACD881A8639E81BD68820409C6E30D142A",
			"4FEED486777ED38E44C489C7C4E93A830E4C4A907FA19A174E630EF0F6ED0409",
			"83EE32E4E145024D29BCA54F71FA335A98B3E68283F1A3099C4D4AE113B53E54"
		};
#else
		std::string dataSet[] {
			"ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57",
			"FE9BC2EF8DF88E708CAB471F82B54DBFCBA11B121E7C2D02799AB4D3A53F0E5B",
			"DAEE5A32E12CEDEFD0349FDBA1FCBDB45356CA3A35AA5CF1A8AE1091BBA98B73",
			"A6954BAA315EE50453CCE3483906F134405B8B3ADD94BFC8D8125CF3C09BBFE8",
			"832A237053A83B7E97CA287AC15F9AD5838898E7395967B56D39749652EA25C3"
		};
		std::string expectedSet[] {
			"5C9901721703B1B082263065BDE4929079312FB6A09683C00F131AA794796467",
			"887258790597075D955EC709131255333E5F62327933D236D6160E56A8B75A6D",
			"DBAC0727B529972CF54D0DFAA52928AAA5A99766CB1912EF3B430BD30647EEDE",
			"3A147249DD5DEC2DEBB0787F2B99E6BC5961FFB361600116D88444B461C8EF22",
			"0EF23A8FDC27032AC21065D39B965CACBEBECD08CDAE2E18AB205D751F1E7626"
		};
#endif

		ASSERT_EQ(CountOf(dataSet), CountOf(expectedSet));
		for (size_t i = 0; i < CountOf(dataSet); ++i) {
			// Act:
			auto keyPair = KeyPair::FromString(dataSet[i]);

			// Assert:
			EXPECT_EQ(utils::ParseByteArray<Key>(expectedSet[i]), keyPair.publicKey());
		}
	}
}}
