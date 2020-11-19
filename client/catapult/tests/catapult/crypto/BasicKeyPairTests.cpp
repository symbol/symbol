/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/crypto/BasicKeyPair.h"
#include "catapult/crypto/SecureByteArray.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS BasicKeyPairTests

	namespace {
		// region ConcreteKeyPair

		constexpr auto Default_Private_Key_String = "CBD84EF8F5F38A25C01308785EA99627DE897D151AFDFCDA";
		constexpr auto Default_Public_Key_String = "CCD94FF9F6F48B26C11409795FAA9728DF8A7E161BFEFDDBA0A1A2A3A4A5A6A7";

		struct ConcretePrivateKey_tag { static constexpr size_t Size = 24; };
		using ConcretePrivateKey = SecureByteArray<ConcretePrivateKey_tag>;

		struct ConcretePublicKey_tag { static constexpr size_t Size = 32; };
		using ConcretePublicKey = utils::ByteArray<ConcretePublicKey_tag>;

		struct ConcreteKeyPairTraits {
		public:
			using PublicKey = ConcretePublicKey;
			using PrivateKey = ConcretePrivateKey;

		public:
			static void ExtractPublicKeyFromPrivateKey(const PrivateKey& privateKey, PublicKey& publicKey) {
				for (auto i = 0u; i < PrivateKey::Size; ++i)
					publicKey[i] = static_cast<uint8_t>(privateKey.data()[i] + 1);

				for (auto i = PrivateKey::Size; i < PublicKey::Size; ++i)
					publicKey[i] = static_cast<uint8_t>(0xA0 + i - PrivateKey::Size);
			}
		};

		using ConcreteKeyPair = BasicKeyPair<ConcreteKeyPairTraits>;

		// endregion
	}

	// region FromString

	TEST(TEST_CLASS, CanCreateFromValidString) {
		// Act:
		auto keyPair = ConcreteKeyPair::FromString(Default_Private_Key_String);

		// Assert:
		EXPECT_EQ(ConcretePrivateKey::FromString(Default_Private_Key_String), keyPair.privateKey());
		EXPECT_EQ(utils::ParseByteArray<ConcretePublicKey>(Default_Public_Key_String), keyPair.publicKey());
	}

	TEST(TEST_CLASS, KeyPairsCreatedFromSameStringsMatch) {
		// Arrange:
		auto seed = test::GenerateRandomHexString(2 * ConcretePrivateKey::Size);

		// Act:
		auto keyPair1 = ConcreteKeyPair::FromString(seed);
		auto keyPair2 = ConcreteKeyPair::FromString(seed);

		// Assert:
		EXPECT_EQ(keyPair1.privateKey(), keyPair2.privateKey());
	}

	TEST(TEST_CLASS, KeyPairsCreatedFromDifferentStringsDoNotMatch) {
		// Arrange:
		auto seed1 = test::GenerateRandomHexString(2 * ConcretePrivateKey::Size);
		auto seed2 = test::GenerateRandomHexString(2 * ConcretePrivateKey::Size);

		// Act:
		auto keyPair1 = ConcreteKeyPair::FromString(seed1);
		auto keyPair2 = ConcreteKeyPair::FromString(seed2);

		// Assert:
		EXPECT_NE(keyPair1.privateKey(), keyPair2.privateKey());
	}

	namespace {
		void AssertCannotCreateFromStringWithSize(size_t size, char keyFirstChar) {
			// Arrange:
			auto rawKeyString = test::GenerateRandomHexString(2 * size);
			rawKeyString[0] = keyFirstChar;

			// Act + Assert: key creation should fail but string should not be cleared
			EXPECT_THROW(ConcreteKeyPair::FromString(rawKeyString), catapult_invalid_argument) << "string size: " << size;
			EXPECT_EQ(keyFirstChar, rawKeyString[0]);
		}
	}

	TEST(TEST_CLASS, CannotCreateFromInvalidString) {
		AssertCannotCreateFromStringWithSize(ConcretePublicKey::Size, 'a');

		AssertCannotCreateFromStringWithSize(ConcretePrivateKey::Size - 1, 'a');
		AssertCannotCreateFromStringWithSize(ConcretePrivateKey::Size, 'g');
		AssertCannotCreateFromStringWithSize(ConcretePrivateKey::Size + 1, 'a');
	}

	// endregion

	// region FromPrivateKey

	TEST(TEST_CLASS, CanCreateFromPrivateKey) {
		// Act:
		auto keyPair = ConcreteKeyPair::FromPrivate(ConcretePrivateKey::FromString(Default_Private_Key_String));

		// Assert:
		EXPECT_EQ(ConcretePrivateKey::FromString(Default_Private_Key_String), keyPair.privateKey());
		EXPECT_EQ(utils::ParseByteArray<ConcretePublicKey>(Default_Public_Key_String), keyPair.publicKey());
	}

	TEST(TEST_CLASS, KeyPairsCreatedFromSamePrivateKeysMatch) {
		// Arrange:
		auto seed = test::GenerateRandomVector(ConcretePrivateKey::Size);

		// Act:
		auto keyPair1 = ConcreteKeyPair::FromPrivate(ConcretePrivateKey::FromBuffer(seed));
		auto keyPair2 = ConcreteKeyPair::FromPrivate(ConcretePrivateKey::FromBuffer(seed));

		// Assert:
		EXPECT_EQ(keyPair1.privateKey(), keyPair2.privateKey());
	}

	TEST(TEST_CLASS, KeyPairsCreatedFromDifferentPrivateKeysDoNotMatch) {
		// Arrange:
		auto seed1 = test::GenerateRandomVector(ConcretePrivateKey::Size);
		auto seed2 = test::GenerateRandomVector(ConcretePrivateKey::Size);

		// Act:
		auto keyPair1 = ConcreteKeyPair::FromPrivate(ConcretePrivateKey::FromBuffer(seed1));
		auto keyPair2 = ConcreteKeyPair::FromPrivate(ConcretePrivateKey::FromBuffer(seed2));

		// Assert:
		EXPECT_NE(keyPair1.privateKey(), keyPair2.privateKey());
	}

	TEST(TEST_CLASS, KeyPairCreatedFromPrivateKeyMatchesKeyPairCreatedFromString) {
		// Act:
		auto keyPair1 = ConcreteKeyPair::FromPrivate(ConcretePrivateKey::FromString(Default_Private_Key_String));
		auto keyPair2 = ConcreteKeyPair::FromString(Default_Private_Key_String);

		// Assert:
		EXPECT_EQ(keyPair1.privateKey(), keyPair2.privateKey());
	}

	// endregion
}}
