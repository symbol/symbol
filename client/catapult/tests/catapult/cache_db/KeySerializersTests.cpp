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

#include "catapult/cache_db/KeySerializers.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS KeySerializersTests

	TEST(TEST_CLASS, CanSerializeBaseValue) {
		// Arrange:
		auto key = test::GenerateRandomValue<Height>();

		// Act:
		auto result = SerializeKey(key);

		// Assert:
		ASSERT_EQ(sizeof(Height), result.Size);
		EXPECT_EQ(test::AsBytePointer(&key), result.pData);
	}

	TEST(TEST_CLASS, CanSerializeArray) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto result = SerializeKey(key);

		// Assert:
		ASSERT_EQ(Hash256::Size, result.Size);
		EXPECT_EQ(key.data(), result.pData);
	}
}}
