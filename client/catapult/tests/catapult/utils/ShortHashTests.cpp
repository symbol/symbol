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

#include "catapult/utils/ShortHash.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS ShortHashTests

	// region hasher

	TEST(TEST_CLASS, HasherIsDeterministic) {
		// Arrange:
		ShortHashHasher hasher;
		auto shortHash = ShortHash(123456789);

		// Act:
		auto result1 = hasher(shortHash);
		auto result2 = hasher(shortHash);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, HashHasSameValueAsShortHash) {
		// Arrange:
		ShortHashHasher hasher;
		auto shortHash = ShortHash(123456789);

		// Act:
		auto result = hasher(shortHash);

		// Assert:
		EXPECT_EQ(123456789u, result);
	}

	TEST(TEST_CLASS, EqualShortHashesHaveSameHash) {
		// Arrange:
		ShortHashHasher hasher;
		auto shortHash1 = ShortHash(123456789);
		auto shortHash2 = ShortHash(123456789);

		// Act:
		auto result1 = hasher(shortHash1);
		auto result2 = hasher(shortHash2);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, DifferentShortHashesHaveDifferentHashes) {
		// Arrange:
		ShortHashHasher hasher;
		auto shortHash1 = ShortHash(123456789);
		auto shortHash2 = ShortHash(987654321);

		// Act:
		auto result1 = hasher(shortHash1);
		auto result2 = hasher(shortHash2);

		// Assert:
		EXPECT_NE(result1, result2);
	}

	// endregion

	// region ToShortHash

	TEST(TEST_CLASS, CanConvertHashToShortHash) {
		// Arrange:
		Hash256 hash;
		ParseHexStringIntoContainer("031729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31C", 2 * Hash256::Size, hash);

		// Act:
		auto shortHash = ToShortHash(hash);

		// Assert:
		EXPECT_EQ(ShortHash(0xD1291703), shortHash);
	}

	// endregion
}}
