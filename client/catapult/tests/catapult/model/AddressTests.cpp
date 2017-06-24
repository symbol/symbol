#include "catapult/model/Address.h"
#include "catapult/model/NetworkInfo.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	namespace {
		constexpr auto Network_Identifier = NetworkIdentifier::Mijin_Test;
		constexpr auto Encoded_Address = "SAAA244WMCB2JXGNQTQHQOS45TGBFF4V2MJBVOUI";
		constexpr auto Decoded_Address = "90000D73966083A4DCCD84E0783A5CECCC129795D3121ABA88";
		constexpr auto Public_Key = "75D8BB873DA8F5CCA741435DE76A46AFC2840803EBF080E931195B048D77F88C";

		Key ParseKey(const std::string& publicKeyString) {
			Key publicKey;
			utils::ParseHexStringIntoContainer(publicKeyString.c_str(), 2 * Key_Size, publicKey);
			return publicKey;
		}

		void AssertCannotCreateAddress(const std::string& encoded) {
			// Assert:
			EXPECT_FALSE(IsValidEncodedAddress(encoded, Network_Identifier)) << encoded;
			EXPECT_THROW(StringToAddress(encoded), catapult_runtime_error) << encoded;
		}
	}

	// region StringToAddress

	TEST(AddressTests, CanCreateAddressFromValidEncodedAddress) {
		// Arrange:
		auto encoded = Encoded_Address;
		auto expectedHex = Decoded_Address;

		// Act:
		auto decoded = StringToAddress(encoded);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded, Network_Identifier));
		EXPECT_EQ(expectedHex, test::ToHexString(decoded));
	}

	TEST(AddressTests, CannotCreateAddressFromEncodedStringWithWrongLength) {
		// Assert:
		AssertCannotCreateAddress(std::string(Encoded_Address) + "ABCDEFGH");
	}

	TEST(AddressTests, CannotCreateAddressFromInvalidEncodedString) {
		// Arrange:
		auto encoded = std::string(Encoded_Address);

		// Assert: set invalid chars
		for (auto ch : { '(', '1', '?' }) {
			encoded[4] = ch;
			AssertCannotCreateAddress(encoded);
		}
	}

	// endregion

	// region AddressToString

	TEST(AddressTests, CanCreateEncodedAddressFromAddress) {
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

	TEST(AddressTests, CanCreateAddressFromPublicKeyForWellKnownNetwork) {
		// Arrange:
		auto expected = "60000D73966083A4DCCD84E0783A5CECCC129795D32534F0A7";
		auto publicKey = ParseKey(Public_Key);
		auto networkId = NetworkIdentifier::Mijin;

		// Act:
		auto decoded = PublicKeyToAddress(publicKey, networkId);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded, NetworkIdentifier::Mijin));
		EXPECT_EQ(decoded[0], utils::to_underlying_type(networkId));
		EXPECT_EQ(expected, test::ToHexString(decoded));
	}

	TEST(AddressTests, CanCreateAddressFromPublicKeyForCustomNetwork) {
		// Arrange:
		auto expected = "7B000D73966083A4DCCD84E0783A5CECCC129795D3D6A7CE45";
		auto publicKey = ParseKey(Public_Key);
		auto networkId = static_cast<NetworkIdentifier>(123);

		// Act:
		auto decoded = PublicKeyToAddress(publicKey, networkId);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded, networkId));
		EXPECT_EQ(decoded[0], utils::to_underlying_type(networkId));
		EXPECT_EQ(expected, test::ToHexString(decoded));
	}

	TEST(AddressTests, AddressCalculationIsDeterministic) {
		// Arrange:
		auto publicKey = ParseKey(Public_Key);

		// Act:
		auto decoded1 = PublicKeyToAddress(publicKey, Network_Identifier);
		auto decoded2 = PublicKeyToAddress(publicKey, Network_Identifier);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded1, Network_Identifier));
		EXPECT_EQ(decoded1, decoded2);
	}

	TEST(AddressTests, DifferentPublicKeysResultInDifferentAddresses) {
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

	TEST(AddressTests, DifferentNetworksResultInDifferentAddresses) {
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

	TEST(AddressTests, IsValidAddressReturnsTrueForValidAddress) {
		// Arrange:
		auto decoded = test::ToArray<Address_Decoded_Size>(Decoded_Address);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded, Network_Identifier));
	}

	TEST(AddressTests, IsValidAddressReturnsFalseForWrongNetworkAddress) {
		// Arrange:
		auto decoded = test::ToArray<Address_Decoded_Size>(Decoded_Address);

		// Assert:
		EXPECT_FALSE(IsValidAddress(decoded, static_cast<NetworkIdentifier>(123)));
	}

	TEST(AddressTests, IsValidAddressReturnsFalseForAddressWithInvalidChecksum) {
		// Arrange:
		auto decoded = test::ToArray<Address_Decoded_Size>(Decoded_Address);
		decoded[Address_Decoded_Size - 1] ^= 0xff; // ruin checksum

		// Assert:
		EXPECT_FALSE(IsValidAddress(decoded, Network_Identifier));
	}

	TEST(AddressTests, IsValidAddressReturnsFalseForAddressWithInvalidHash) {
		// Arrange:
		auto decoded = test::ToArray<Address_Decoded_Size>(Decoded_Address);
		decoded[5] ^= 0xff; // ruin ripemd160 hash

		// Assert:
		EXPECT_FALSE(IsValidAddress(decoded, Network_Identifier));
	}

	// endregion

	// region IsValidEncodedAddress

	TEST(AddressTests, IsValidEncodedAddressReturnsTrueForValidEncodedAddress) {
		// Arrange:
		auto encoded = Encoded_Address;

		// Assert:
		EXPECT_TRUE(IsValidEncodedAddress(encoded, Network_Identifier));
	}

	TEST(AddressTests, IsValidEncodedAddressReturnsFalseForWrongNetworkAddress) {
		// Arrange:
		auto encoded = Encoded_Address;

		// Assert:
		EXPECT_FALSE(IsValidEncodedAddress(encoded, static_cast<NetworkIdentifier>(123)));
	}

	TEST(AddressTests, IsValidEncodedAddressReturnsFalseForInvalidEncodedAddress) {
		// Arrange: change last char of valid address
		auto encoded = std::string(Encoded_Address);
		++encoded.back();

		// Assert:
		EXPECT_FALSE(IsValidEncodedAddress(encoded, Network_Identifier));
	}

	TEST(AddressTests, IsValidEncodedAddressReturnsFalseForEncodedAddressWithWrongLength) {
		// Arrange: add additional characters to the end of a valid address
		auto encoded = std::string(Encoded_Address);
		encoded += "ABC";

		// Assert:
		EXPECT_FALSE(IsValidEncodedAddress(encoded, Network_Identifier));
	}

	TEST(AddressTests, AddingLeadingOrTrailingWhiteSpaceInvalidatesEncodedAddress) {
		// Arrange:
		auto encoded = std::string(Encoded_Address);

		// Assert:
		EXPECT_FALSE(IsValidEncodedAddress("   \t    " + encoded, Network_Identifier));
		EXPECT_FALSE(IsValidEncodedAddress(encoded + "   \t    ", Network_Identifier));
		EXPECT_FALSE(IsValidEncodedAddress("   \t    " + encoded + "   \t    ", Network_Identifier));
	}

	// endregion
}}
