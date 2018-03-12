#include "catapult/crypto/KeyPair.h"
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
		AssertCannotCreatePrivateKeyFromStringWithSize(Key_Size * 1, 'a');
		AssertCannotCreatePrivateKeyFromStringWithSize(Key_Size * 2, 'g');
		AssertCannotCreatePrivateKeyFromStringWithSize(Key_Size * 3, 'a');
	}

	TEST(TEST_CLASS, CanCreateKeyPairFromValidString) {
		// Arrange:
#ifdef SIGNATURE_SCHEME_NIS1
		auto rawKeyString = std::string("3485D98EFD7EB07ADAFCFD1A157D89DE2796A95E780813C0258AF3F5F84ED8CB");
		auto expectedKey = std::string("C54D6E33ED1446EEDD7F7A80A588DD01857F723687A09200C1917D5524752F8B");
#else
		auto rawKeyString = std::string("CBD84EF8F5F38A25C01308785EA99627DE897D151AFDFCDA7AB07EFD8ED98534");
		auto expectedKey = std::string("A6DC1C33C26BC67B21AC4B3F4D1E88901E23AD208260F40AF3CB0A6CE9557852");
#endif
		// Act:
		auto keyPair = KeyPair::FromString(rawKeyString);

		// Assert:
		EXPECT_EQ(expectedKey, test::ToHexString(keyPair.publicKey()));
		EXPECT_EQ(rawKeyString, test::ToHexString(keyPair.privateKey().data(), keyPair.privateKey().size()));
	}

	TEST(TEST_CLASS, CanCreateKeyPairFromPrivateKey) {
		// Arrange:
		auto privateKeyStr = test::GenerateRandomHexString(Key_Size * 2);
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
		auto privateKeyStr = std::string("3485d98efd7eb07adafcfd1a157d89de2796a95e780813c0258af3f5f84ed8cb");
		auto privateKey = PrivateKey::FromString(privateKeyStr);

		// Act:
		auto keyPair1 = KeyPair::FromPrivate(std::move(privateKey));
		auto keyPair2 = KeyPair::FromString(privateKeyStr);

		// Assert:
		EXPECT_EQ(keyPair1.privateKey(), keyPair2.privateKey());
	}

	TEST(TEST_CLASS, PassesNemTestVectors) {
		// Arrange:
#ifdef SIGNATURE_SCHEME_NIS1
		// from nem https://github.com/NewEconomyMovement/nem-test-vectors)
		std::string dataSet[] {
			"575dbb3062267eff57c970a336ebbc8fbcfe12c5bd3ed7bc11eb0481d7704ced",
			"5b0e3fa5d3b49a79022d7c1e121ba1cbbf4db5821f47ab8c708ef88defc29bfe",
			"738ba9bb9110aea8f15caa353aca5653b4bdfca1db9f34d0efed2ce1325aeeda",
			"e8bf9bc0f35c12d8c8bf94dd3a8b5b4034f1063948e3cc5304e55e31aa4b95a6",
			"c325ea529674396db5675939e7988883d59a5fc17a28ca977e3ba85370232a83"
		};

		std::string expectedSet[] {
			"C5F54BA980FCBB657DBAAA42700539B207873E134D2375EFEAB5F1AB52F87844",
			"96EB2A145211B1B7AB5F0D4B14F8ABC8D695C7AEE31A3CFC2D4881313C68EEA3",
			"2D8425E4CA2D8926346C7A7CA39826ACD881A8639E81BD68820409C6E30D142A",
			"4FEED486777ED38E44C489C7C4E93A830E4C4A907FA19A174E630EF0F6ED0409",
			"83EE32E4E145024D29BCA54F71FA335A98B3E68283F1A3099C4D4AE113B53E54",
		};
#else
		std::string dataSet[] {
			"ed4c70d78104eb11bcd73ebdc512febc8fbceb36a370c957ff7e266230bb5d57",
			"fe9bc2ef8df88e708cab471f82b54dbfcba11b121e7c2d02799ab4d3a53f0e5b",
			"daee5a32e12cedefd0349fdba1fcbdb45356ca3a35aa5cf1a8ae1091bba98b73",
			"a6954baa315ee50453cce3483906f134405b8b3add94bfc8d8125cf3c09bbfe8",
			"832a237053a83b7e97ca287ac15f9ad5838898e7395967b56d39749652ea25c3"
		};
		std::string expectedSet[] {
			"5C9901721703B1B082263065BDE4929079312FB6A09683C00F131AA794796467",
			"887258790597075D955EC709131255333E5F62327933D236D6160E56A8B75A6D",
			"DBAC0727B529972CF54D0DFAA52928AAA5A99766CB1912EF3B430BD30647EEDE",
			"3A147249DD5DEC2DEBB0787F2B99E6BC5961FFB361600116D88444B461C8EF22",
			"0EF23A8FDC27032AC21065D39B965CACBEBECD08CDAE2E18AB205D751F1E7626",
		};
#endif

		ASSERT_EQ(CountOf(dataSet), CountOf(expectedSet));
		for (size_t i = 0; i < CountOf(dataSet); ++i) {
			// Act:
			auto keyPair = KeyPair::FromString(dataSet[i]);

			// Assert:
			EXPECT_EQ(expectedSet[i], test::ToHexString(keyPair.publicKey()));
		}
	}
}}
