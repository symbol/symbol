#include "catapult/model/Address.h"
#include "catapult/model/NetworkInfo.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS AddressTests

	namespace {
		constexpr auto Network_Identifier = NetworkIdentifier::Mijin_Test;
#ifdef SIGNATURE_SCHEME_NIS1
		constexpr auto Encoded_Address = "SCG7O6OEEMP6M6OHXBQJYYEZYQ7ZMWW2NJ3PKISE";
		constexpr auto Decoded_Address = "908DF779C4231FE679C7B8609C6099C43F965ADA6A76F52244";
#else
		constexpr auto Encoded_Address = "SAAA244WMCB2JXGNQTQHQOS45TGBFF4V2MJBVOUI";
		constexpr auto Decoded_Address = "90000D73966083A4DCCD84E0783A5CECCC129795D3121ABA88";
#endif
		constexpr auto Public_Key = "75D8BB873DA8F5CCA741435DE76A46AFC2840803EBF080E931195B048D77F88C";

		Key ParseKey(const std::string& publicKeyString) {
			Key publicKey;
			utils::ParseHexStringIntoContainer(publicKeyString.c_str(), 2 * Key_Size, publicKey);
			return publicKey;
		}

		void AssertCannotCreateAddress(const std::string& encoded) {
			// Act + Assert:
			EXPECT_FALSE(IsValidEncodedAddress(encoded, Network_Identifier)) << encoded;
			EXPECT_THROW(StringToAddress(encoded), catapult_runtime_error) << encoded;
		}
	}

	// region StringToAddress

	TEST(TEST_CLASS, CanCreateAddressFromValidEncodedAddress) {
		// Arrange:
		auto encoded = Encoded_Address;
		auto expectedHex = Decoded_Address;

		// Act:
		auto decoded = StringToAddress(encoded);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded, Network_Identifier));
		EXPECT_EQ(expectedHex, test::ToHexString(decoded));
	}

	TEST(TEST_CLASS, CannotCreateAddressFromEncodedStringWithWrongLength) {
		// Assert:
		AssertCannotCreateAddress(std::string(Encoded_Address) + "ABCDEFGH");
	}

	TEST(TEST_CLASS, CannotCreateAddressFromInvalidEncodedString) {
		// Arrange:
		auto encoded = std::string(Encoded_Address);

		// Act + Assert: set invalid chars
		for (auto ch : { '(', '1', '?' }) {
			encoded[4] = ch;
			AssertCannotCreateAddress(encoded);
		}
	}

	// endregion

	// region AddressToString

	TEST(TEST_CLASS, CanCreateEncodedAddressFromAddress) {
		// Arrange:
		auto decodedHex = Decoded_Address;
		auto expected = Encoded_Address;

		// Act:
		auto encoded = AddressToString(test::ToArray<Address_Decoded_Size>(decodedHex));

		// Assert:
		EXPECT_TRUE(IsValidEncodedAddress(encoded, Network_Identifier));
		EXPECT_EQ(expected, encoded);
	}

	// endregion

	// region PublicKeyToAddress

	TEST(TEST_CLASS, CanCreateAddressFromPublicKeyForWellKnownNetwork) {
		// Arrange:
#ifdef SIGNATURE_SCHEME_NIS1
		auto expected = "60E4DF097CBFD7E1E216C0E84BD4F524E28DA80D5C35EC4431";
#else
		auto expected = "60000D73966083A4DCCD84E0783A5CECCC129795D32534F0A7";
#endif
		auto publicKey = ParseKey(Public_Key);
		auto networkId = NetworkIdentifier::Mijin;

		// Act:
		auto decoded = PublicKeyToAddress(publicKey, networkId);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded, NetworkIdentifier::Mijin));
		EXPECT_EQ(decoded[0], utils::to_underlying_type(networkId));
		EXPECT_EQ(expected, test::ToHexString(decoded));
	}

	TEST(TEST_CLASS, CanCreateAddressFromPublicKeyForCustomNetwork) {
		// Arrange:
#ifdef SIGNATURE_SCHEME_NIS1
		auto expected = "7BE4DF097CBFD7E1E216C0E84BD4F524E28DA80D5CB68B8A77";
#else
		auto expected = "7B000D73966083A4DCCD84E0783A5CECCC129795D3D6A7CE45";
#endif
		auto publicKey = ParseKey(Public_Key);
		auto networkId = static_cast<NetworkIdentifier>(123);

		// Act:
		auto decoded = PublicKeyToAddress(publicKey, networkId);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded, networkId));
		EXPECT_EQ(decoded[0], utils::to_underlying_type(networkId));
		EXPECT_EQ(expected, test::ToHexString(decoded));
	}

	TEST(TEST_CLASS, AddressCalculationIsDeterministic) {
		// Arrange:
		auto publicKey = ParseKey(Public_Key);

		// Act:
		auto decoded1 = PublicKeyToAddress(publicKey, Network_Identifier);
		auto decoded2 = PublicKeyToAddress(publicKey, Network_Identifier);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded1, Network_Identifier));
		EXPECT_EQ(decoded1, decoded2);
	}

	TEST(TEST_CLASS, DifferentPublicKeysResultInDifferentAddresses) {
		// Arrange:
		auto pubKey1 = test::GenerateRandomData<Key_Size>();
		auto pubKey2 = test::GenerateRandomData<Key_Size>();

		// Act:
		auto decoded1 = PublicKeyToAddress(pubKey1, Network_Identifier);
		auto decoded2 = PublicKeyToAddress(pubKey2, Network_Identifier);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded1, Network_Identifier));
		EXPECT_TRUE(IsValidAddress(decoded2, Network_Identifier));
		EXPECT_NE(decoded1, decoded2);
	}

	TEST(TEST_CLASS, DifferentNetworksResultInDifferentAddresses) {
		// Arrange:
		auto pubKey = test::GenerateRandomData<Key_Size>();

		// Act:
		auto decoded1 = PublicKeyToAddress(pubKey, NetworkIdentifier::Public);
		auto decoded2 = PublicKeyToAddress(pubKey, NetworkIdentifier::Public_Test);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded1, NetworkIdentifier::Public));
		EXPECT_TRUE(IsValidAddress(decoded2, NetworkIdentifier::Public_Test));
		EXPECT_NE(decoded1, decoded2);
	}

	// endregion

	// region IsValidAddress

	TEST(TEST_CLASS, IsValidAddressReturnsTrueForValidAddress) {
		// Arrange:
		auto decoded = test::ToArray<Address_Decoded_Size>(Decoded_Address);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded, Network_Identifier));
	}

	TEST(TEST_CLASS, IsValidAddressReturnsFalseForWrongNetworkAddress) {
		// Arrange:
		auto decoded = test::ToArray<Address_Decoded_Size>(Decoded_Address);

		// Assert:
		EXPECT_FALSE(IsValidAddress(decoded, static_cast<NetworkIdentifier>(123)));
	}

	TEST(TEST_CLASS, IsValidAddressReturnsFalseForAddressWithInvalidChecksum) {
		// Arrange:
		auto decoded = test::ToArray<Address_Decoded_Size>(Decoded_Address);
		decoded[Address_Decoded_Size - 1] ^= 0xFF; // ruin checksum

		// Assert:
		EXPECT_FALSE(IsValidAddress(decoded, Network_Identifier));
	}

	TEST(TEST_CLASS, IsValidAddressReturnsFalseForAddressWithInvalidHash) {
		// Arrange:
		auto decoded = test::ToArray<Address_Decoded_Size>(Decoded_Address);
		decoded[5] ^= 0xFF; // ruin ripemd160 hash

		// Assert:
		EXPECT_FALSE(IsValidAddress(decoded, Network_Identifier));
	}

	// endregion

	// region IsValidEncodedAddress

	TEST(TEST_CLASS, IsValidEncodedAddressReturnsTrueForValidEncodedAddress) {
		// Arrange:
		auto encoded = Encoded_Address;

		// Assert:
		EXPECT_TRUE(IsValidEncodedAddress(encoded, Network_Identifier));
	}

	TEST(TEST_CLASS, IsValidEncodedAddressReturnsFalseForWrongNetworkAddress) {
		// Arrange:
		auto encoded = Encoded_Address;

		// Assert:
		EXPECT_FALSE(IsValidEncodedAddress(encoded, static_cast<NetworkIdentifier>(123)));
	}

	TEST(TEST_CLASS, IsValidEncodedAddressReturnsFalseForInvalidEncodedAddress) {
		// Arrange: change last char of valid address
		auto encoded = std::string(Encoded_Address);
		++encoded.back();

		// Assert:
		EXPECT_FALSE(IsValidEncodedAddress(encoded, Network_Identifier));
	}

	TEST(TEST_CLASS, IsValidEncodedAddressReturnsFalseForEncodedAddressWithWrongLength) {
		// Arrange: add additional characters to the end of a valid address
		auto encoded = std::string(Encoded_Address);
		encoded += "ABC";

		// Assert:
		EXPECT_FALSE(IsValidEncodedAddress(encoded, Network_Identifier));
	}

	TEST(TEST_CLASS, AddingLeadingOrTrailingWhiteSpaceInvalidatesEncodedAddress) {
		// Arrange:
		auto encoded = std::string(Encoded_Address);

		// Assert:
		EXPECT_FALSE(IsValidEncodedAddress("   \t    " + encoded, Network_Identifier));
		EXPECT_FALSE(IsValidEncodedAddress(encoded + "   \t    ", Network_Identifier));
		EXPECT_FALSE(IsValidEncodedAddress("   \t    " + encoded + "   \t    ", Network_Identifier));
	}

	// endregion
}}
