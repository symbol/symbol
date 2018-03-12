#include "catapult/model/NetworkInfo.h"
#include "catapult/utils/Casting.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS NetworkInfoTests

	// region construction

	TEST(TEST_CLASS, CanCreateDefaultNetwork) {
		// Act:
		NetworkInfo info;

		// Assert:
		EXPECT_EQ(0x00u, utils::to_underlying_type(info.Identifier));
		EXPECT_EQ(Key{}, info.PublicKey);
		EXPECT_EQ(Hash256{}, info.GenerationHash);
	}

	TEST(TEST_CLASS, CanCreateCustomNetwork) {
		// Arrange:
		auto publicKey = test::GenerateRandomData<Key_Size>();
		auto generationHash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		NetworkInfo info(static_cast<NetworkIdentifier>(0xB9), publicKey, generationHash);

		// Assert:
		EXPECT_EQ(0xB9u, utils::to_underlying_type(info.Identifier));
		EXPECT_EQ(publicKey, info.PublicKey);
		EXPECT_EQ(generationHash, info.GenerationHash);
	}

	// endregion

	// region parsing

	TEST(TEST_CLASS, CanParseValidNetworkValue) {
		// Arrange:
		auto assertSuccessfulParse = [](const auto& input, const auto& expectedParsedValue) {
			test::AssertParse(input, expectedParsedValue, [](const auto& str, auto& parsedValue) {
				return TryParseValue(str, parsedValue);
			});
		};

		// Assert:
		assertSuccessfulParse("mijin", NetworkIdentifier::Mijin);
		assertSuccessfulParse("mijin-test", NetworkIdentifier::Mijin_Test);
		assertSuccessfulParse("public", NetworkIdentifier::Public);
		assertSuccessfulParse("public-test", NetworkIdentifier::Public_Test);

		assertSuccessfulParse("0", static_cast<NetworkIdentifier>(0));
		assertSuccessfulParse("17", static_cast<NetworkIdentifier>(17));
		assertSuccessfulParse("255", static_cast<NetworkIdentifier>(255));
	}

	TEST(TEST_CLASS, CannotParseInvalidNetworkValue) {
		// Assert:
		test::AssertEnumParseFailure("mijin", NetworkIdentifier::Public, [](const auto& str, auto& parsedValue) {
			return TryParseValue(str, parsedValue);
		});
		test::AssertFailedParse("256", NetworkIdentifier::Public, [](const auto& str, auto& parsedValue) {
			return TryParseValue(str, parsedValue);
		});
	}

	// endregion
}}
