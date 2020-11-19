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

#include "catapult/cache/PatriciaTreeEncoderAdapters.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS PatriciaTreeEncoderAdaptersTests

	namespace {
		class Serializer {
		public:
			using KeyType = Amount;
			using ValueType = std::vector<uint8_t>;

		public:
			static std::string SerializeValue(const ValueType& value) {
				std::string str;
				for (auto byte : value)
					str.push_back(static_cast<char>(byte));

				return str;
			}
		};

		using PlainKeyEncoder = SerializerPlainKeyEncoder<Serializer>;
		using HashedKeyEncoder = SerializerHashedKeyEncoder<Serializer>;

		template<typename TEncoder>
		void AssertCanEncodeValue() {
			// Act:
			auto value = test::GenerateRandomVector(45);
			const auto& encodedValue = TEncoder::EncodeValue(value);

			// Assert:
			Hash256 valueHash;
			crypto::Sha3_256(value, valueHash);

			EXPECT_EQ(valueHash, encodedValue);
		}
	}

	// region SerializerPlainKeyEncoder

	TEST(TEST_CLASS, CanEncodeKey_PlainKey) {
		// Act:
		auto key = Amount(123);
		const auto& encodedKey = PlainKeyEncoder::EncodeKey(key);

		// Assert:
		EXPECT_EQ(&key, &encodedKey);
		EXPECT_EQ(Amount(123), encodedKey);
	}

	TEST(TEST_CLASS, CanEncodeValue_PlainKey) {
		AssertCanEncodeValue<PlainKeyEncoder>();
	}

	// endregion

	// region SerializerHashedKeyEncoder

	TEST(TEST_CLASS, CanEncodeKey_HashedKey) {
		// Act:
		auto key = Amount(123);
		const auto& encodedKey = HashedKeyEncoder::EncodeKey(key);

		// Assert:
		Hash256 keyHash;
		crypto::Sha3_256({ test::AsBytePointer(&key), sizeof(Amount) }, keyHash);

		EXPECT_EQ(keyHash, encodedKey);
	}

	TEST(TEST_CLASS, CanEncodeValue_HashedKey) {
		AssertCanEncodeValue<HashedKeyEncoder>();
	}

	// endregion
}}
