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

#include "catapult/utils/ArraySet.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS ArraySetTests

	TEST(TEST_CLASS, ArrayPointerHasher_ReturnsSameHashOnlyWhenPointedToHashesAreEqual) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto hashCopy = hash;
		auto differentHash = test::GenerateRandomByteArray<Hash256>();
		ArrayPointerHasher<Hash256> hasher;

		// Act + Assert:
		EXPECT_EQ(hasher(&hash), hasher(&hash));
		EXPECT_EQ(hasher(&hash), hasher(&hashCopy));
		EXPECT_NE(hasher(&hash), hasher(&differentHash));
	}

	TEST(TEST_CLASS, ArrayPointerEquality_ReturnsTrueOnlyWhenPointedToHashesAreEqual) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto hashCopy = hash;
		auto differentHash = test::GenerateRandomByteArray<Hash256>();
		ArrayPointerEquality<Hash256> equality;

		// Act + Assert:
		EXPECT_TRUE(equality(&hash, &hash));
		EXPECT_TRUE(equality(&hash, &hashCopy));
		EXPECT_TRUE(equality(&hashCopy, &hash));
		EXPECT_FALSE(equality(&hash, &differentHash));
		EXPECT_FALSE(equality(&differentHash, &hash));
	}
}}
