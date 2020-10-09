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

#include "catapult/model/Address.h"
#include "catapult/model/NetworkIdentifier.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS AddressTests

	namespace {
		constexpr auto Network_Identifier = NetworkIdentifier::Private_Test;
		constexpr auto Encoded_Address = "QAAA244WMCB2JXGNQTQHQOS45TGBFF4V2ONA6DI";
		constexpr auto Decoded_Address = "80000D73966083A4DCCD84E0783A5CECCC129795D39A0F0D";
		constexpr auto Public_Key = "75D8BB873DA8F5CCA741435DE76A46AFC2840803EBF080E931195B048D77F88C";

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
		auto expectedDecoded = utils::ParseByteArray<Address>(Decoded_Address);

		// Act:
		auto decoded = StringToAddress(encoded);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded, Network_Identifier));
		EXPECT_EQ(expectedDecoded, decoded);
	}

	TEST(TEST_CLASS, CannotCreateAddressFromEncodedStringWithWrongLength) {
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

	// region AddressToString / PublicKeyToAddressString

	TEST(TEST_CLASS, CanCreateEncodedAddressFromAddress) {
		// Arrange:
		auto decodedHex = Decoded_Address;
		auto expected = Encoded_Address;

		// Act:
		auto encoded = AddressToString(utils::ParseByteArray<Address>(decodedHex));

		// Assert:
		EXPECT_TRUE(IsValidEncodedAddress(encoded, Network_Identifier));
		EXPECT_EQ(expected, encoded);
	}

	TEST(TEST_CLASS, CanCreateEncodedAddressFromPublicKey) {
		// Arrange:
		auto expected = Encoded_Address;
		auto publicKey = utils::ParseByteArray<Key>(Public_Key);
		auto networkIdentifier = NetworkIdentifier::Private_Test;

		// Act:
		auto encoded = PublicKeyToAddressString(publicKey, networkIdentifier);

		// Assert:
		EXPECT_TRUE(IsValidEncodedAddress(encoded, Network_Identifier));
		EXPECT_EQ(expected, encoded);
	}

	// endregion

	// region PublicKeyToAddress

	TEST(TEST_CLASS, CanCreateAddressFromPublicKeyForWellKnownNetwork) {
		// Arrange:
		auto expected = utils::ParseByteArray<Address>("78000D73966083A4DCCD84E0783A5CECCC129795D3878B85");
		auto publicKey = utils::ParseByteArray<Key>(Public_Key);
		auto networkIdentifier = NetworkIdentifier::Private;

		// Act:
		auto decoded = PublicKeyToAddress(publicKey, networkIdentifier);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded, NetworkIdentifier::Private));
		EXPECT_EQ(decoded[0], utils::to_underlying_type(networkIdentifier));
		EXPECT_EQ(expected, decoded);
	}

	TEST(TEST_CLASS, CanCreateAddressFromPublicKeyForCustomNetwork) {
		// Arrange:
		auto expected = utils::ParseByteArray<Address>("7B000D73966083A4DCCD84E0783A5CECCC129795D3D6A7CE");
		auto publicKey = utils::ParseByteArray<Key>(Public_Key);
		auto networkIdentifier = static_cast<NetworkIdentifier>(123);

		// Act:
		auto decoded = PublicKeyToAddress(publicKey, networkIdentifier);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded, networkIdentifier));
		EXPECT_EQ(decoded[0], utils::to_underlying_type(networkIdentifier));
		EXPECT_EQ(expected, decoded);
	}

	TEST(TEST_CLASS, AddressCalculationIsDeterministic) {
		// Arrange:
		auto publicKey = utils::ParseByteArray<Key>(Public_Key);

		// Act:
		auto decoded1 = PublicKeyToAddress(publicKey, Network_Identifier);
		auto decoded2 = PublicKeyToAddress(publicKey, Network_Identifier);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded1, Network_Identifier));
		EXPECT_EQ(decoded1, decoded2);
	}

	TEST(TEST_CLASS, DifferentPublicKeysResultInDifferentAddresses) {
		// Arrange:
		auto pubKey1 = test::GenerateRandomByteArray<Key>();
		auto pubKey2 = test::GenerateRandomByteArray<Key>();

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
		auto pubKey = test::GenerateRandomByteArray<Key>();

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
		auto decoded = utils::ParseByteArray<Address>(Decoded_Address);

		// Assert:
		EXPECT_TRUE(IsValidAddress(decoded, Network_Identifier));
	}

	TEST(TEST_CLASS, IsValidAddressReturnsFalseForWrongNetworkAddress) {
		// Arrange:
		auto decoded = utils::ParseByteArray<Address>(Decoded_Address);

		// Assert:
		EXPECT_FALSE(IsValidAddress(decoded, static_cast<NetworkIdentifier>(123)));
	}

	TEST(TEST_CLASS, IsValidAddressReturnsFalseForAddressWithInvalidChecksum) {
		// Arrange:
		auto decoded = utils::ParseByteArray<Address>(Decoded_Address);
		decoded[Address::Size - 1] ^= 0xFF; // ruin checksum

		// Assert:
		EXPECT_FALSE(IsValidAddress(decoded, Network_Identifier));
	}

	TEST(TEST_CLASS, IsValidAddressReturnsFalseForAddressWithInvalidHash) {
		// Arrange:
		auto decoded = utils::ParseByteArray<Address>(Decoded_Address);
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

	namespace {
		void AssertInvalidEncodedAddress(const consumer<std::string&>& mutate) {
			// Arrange:
			auto encoded = std::string(Encoded_Address);
			mutate(encoded);

			// Assert:
			EXPECT_FALSE(IsValidEncodedAddress(encoded, Network_Identifier)) << encoded;
		}
	}

	TEST(TEST_CLASS, IsValidEncodedAddressReturnsFalseForInvalidEncodedAddress) {
		// Arrange: corrupt checksum
		AssertInvalidEncodedAddress([](auto& encoded) { ++encoded[encoded.size() / 2]; });

		// - nonzero trail padding byte
		AssertInvalidEncodedAddress([](auto& encoded) { ++encoded.back(); });
	}

	TEST(TEST_CLASS, IsValidEncodedAddressReturnsFalseForEncodedAddressWithWrongLength) {
		// Arrange: add additional characters to the end of a valid address
		AssertInvalidEncodedAddress([](auto& encoded) { encoded += "ABC"; });
	}

	TEST(TEST_CLASS, IsValidEncodedAddressReturnsFalseForEncodedStringWithLeadingOrTrailingWhiteSpace) {
		AssertInvalidEncodedAddress([](auto& encoded) { encoded = "   \t    " + encoded; });
		AssertInvalidEncodedAddress([](auto& encoded) { encoded = encoded + "   \t    "; });
		AssertInvalidEncodedAddress([](auto& encoded) { encoded = "   \t    " + encoded + "   \t    "; });
	}

	// endregion

	// region TryParseValue

	TEST(TEST_CLASS, TryParseValueCanParseValidEncodedAddress) {
		// Arrange:
		auto encoded = Encoded_Address;
		Address decoded;

		// Act:
		auto result = TryParseValue(encoded, decoded);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(utils::ParseByteArray<Address>(Decoded_Address), decoded);
	}

	namespace {
		void AssertTryParseValueFailure(const consumer<std::string&>& mutate) {
			// Arrange:
			auto encoded = std::string(Encoded_Address);
			Address decoded;

			mutate(encoded);

			// Act:
			auto result = TryParseValue(encoded, decoded);

			// Assert:
			EXPECT_FALSE(result) << encoded;
			EXPECT_EQ(Address(), decoded) << encoded;
		}
	}

	TEST(TEST_CLASS, TryParseValueReturnsFalseForInvalidEncodedAddress) {
		// Arrange: corrupt checksum
		AssertTryParseValueFailure([](auto& encoded) { ++encoded[encoded.size() / 2]; });

		// - nonzero trail padding byte
		AssertTryParseValueFailure([](auto& encoded) { ++encoded.back(); });
	}

	TEST(TEST_CLASS, TryParseValueReturnsFalseForEncodedAddressWithWrongLength) {
		// Arrange: add additional characters to the end of a valid address
		AssertTryParseValueFailure([](auto& encoded) { encoded += "ABC"; });
	}

	TEST(TEST_CLASS, TryParseValueReturnsFalseForEncodedStringWithLeadingOrTrailingWhiteSpace) {
		AssertTryParseValueFailure([](auto& encoded) { encoded = "   \t    " + encoded; });
		AssertTryParseValueFailure([](auto& encoded) { encoded = encoded + "   \t    "; });
		AssertTryParseValueFailure([](auto& encoded) { encoded = "   \t    " + encoded + "   \t    "; });
	}

	// endregion
}}
