#include "catapult/crypto/KeyGenerator.h"
#include "catapult/crypto/PrivateKey.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

	namespace {
		const Key Public_Key_Zero = []() {
			Key zeroKey;
			std::fill(zeroKey.begin(), zeroKey.end(), 0);
			return zeroKey; }();
	}

	TEST(KeyGeneratorTests, CanExtractPublicKeyFromPrivateKey) {
		// Arrange:
		auto privateKey = PrivateKey::Generate(test::RandomByte);
		Key publicKey;

		// Act:
		ExtractPublicKeyFromPrivateKey(privateKey, publicKey);

		// Assert:
		EXPECT_NE(Public_Key_Zero, publicKey);
	}

	TEST(KeyGeneratorTests, CanExtractSamePublicKeyFromSamePrivateKey) {
		// Arrange::
		auto generatePublicKey = []() {
			Key publicKey;
			auto privateKey = PrivateKey::Generate([]() { return 7; });
			ExtractPublicKeyFromPrivateKey(privateKey, publicKey);
			return publicKey;
		};

		// Act:
		auto publicKey1 = generatePublicKey();
		auto publicKey2 = generatePublicKey();

		// Assert:
		EXPECT_EQ(publicKey1, publicKey2);
	}
}}
