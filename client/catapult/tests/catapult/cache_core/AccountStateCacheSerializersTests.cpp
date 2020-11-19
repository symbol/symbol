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

#include "catapult/cache_core/AccountStateCacheSerializers.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountStateCacheSerializersTests

	TEST(TEST_CLASS, KeyAddressPairSerializer_CanSerializeValue) {
		// Arrange:
		auto originalPair = std::make_pair(test::GenerateRandomByteArray<Key>(), test::GenerateRandomByteArray<Address>());

		// Act:
		auto result = KeyAddressPairSerializer::SerializeValue(originalPair);

		// Assert:
		ASSERT_EQ(Key::Size + Address::Size, result.size());
		EXPECT_EQ(originalPair.first, reinterpret_cast<const Key&>(result[0]));
		EXPECT_EQ(originalPair.second, reinterpret_cast<const Address&>(result[Key::Size]));
	}

	TEST(TEST_CLASS, KeyAddressPairSerializer_CanDeserializeValue) {
		// Arrange:
		auto buffer = test::GenerateRandomArray<Key::Size + Address::Size>();

		// Act:
		auto pair = KeyAddressPairSerializer::DeserializeValue(buffer);

		// Assert:
		EXPECT_EQ(reinterpret_cast<const Key&>(buffer[0]), pair.first);
		EXPECT_EQ(reinterpret_cast<const Address&>(buffer[Key::Size]), pair.second);
	}

	TEST(TEST_CLASS, KeyAddressPairSerializer_CanRoundtripValue) {
		// Arrange:
		auto originalPair = std::make_pair(test::GenerateRandomByteArray<Key>(), test::GenerateRandomByteArray<Address>());

		// Act:
		auto result = test::RunRoundtripStringTest<KeyAddressPairSerializer>(originalPair);

		// Assert:
		EXPECT_EQ(originalPair.first, result.first);
		EXPECT_EQ(originalPair.second, result.second);
	}
}}
