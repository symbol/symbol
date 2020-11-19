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

#include "catapult/state/TimestampedHash.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS TimestampedHashTests

	namespace {
		TimestampedHash::HashType GetPartialHash(const Hash256& hash) {
			TimestampedHash::HashType partialHash;
			std::memcpy(partialHash.data(), hash.data(), partialHash.size());
			return partialHash;
		}
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateTimestampedHash) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto timestampedHash = TimestampedHash(Timestamp(123), hash);

		// Assert:
		EXPECT_EQ(Timestamp(123), timestampedHash.Time);
		EXPECT_EQ(GetPartialHash(hash), timestampedHash.Hash);
	}

	TEST(TEST_CLASS, CanCreateTimestampedHashFromTimestamp) {
		// Arrange:
		auto timestampedHash = TimestampedHash(Timestamp(123));

		// Assert:
		EXPECT_EQ(Timestamp(123), timestampedHash.Time);
		EXPECT_EQ(TimestampedHash::HashType(), timestampedHash.Hash);
	}

	TEST(TEST_CLASS, CanCreateDefaultTimestampedHash) {
		// Arrange:
		auto timestampedHash = TimestampedHash();

		// Assert:
		EXPECT_EQ(Timestamp(0), timestampedHash.Time);
		EXPECT_EQ(TimestampedHash::HashType(), timestampedHash.Hash);
	}

	// endregion

	// region comparison operators

	TEST(TEST_CLASS, OperatorLessThanReturnsTrueForSmallerValuesAndFalseOtherwise) {
		// Arrange:
		Hash256 hash1{ {} };
		Hash256 hash2{ { 1 } }; // hash1 < hash2
		std::vector<TimestampedHash> timestampedHashes{
			TimestampedHash(Timestamp(123), hash1),
			TimestampedHash(Timestamp(123), hash2),
			TimestampedHash(Timestamp(234), hash1),
			TimestampedHash(Timestamp(234), hash2)
		};

		// Assert:
		test::AssertLessThanOperatorForEqualValues(TimestampedHash(Timestamp(123), hash1), TimestampedHash(Timestamp(123), hash1));
		test::AssertOperatorBehaviorForIncreasingValues(timestampedHashes, std::less<>());
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueForEqualObjects) {
		// Arrange:
		Hash256 hash1{ {} };
		Hash256 hash2{ { 1 } }; // hash1 < hash2
		auto timestampedHash1 = TimestampedHash(Timestamp(123), hash1);
		auto timestampedHash2 = TimestampedHash(Timestamp(123), hash1);
		auto timestampedHash3 = TimestampedHash(Timestamp(234), hash1);
		auto timestampedHash4 = TimestampedHash(Timestamp(123), hash2);
		auto timestampedHash5 = TimestampedHash(Timestamp(234), hash2);

		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects(
				{ timestampedHash1, timestampedHash2 },
				{ timestampedHash3, timestampedHash4, timestampedHash5 });
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueForUnequalObjects) {
		// Arrange:
		Hash256 hash1{ {} };
		Hash256 hash2{ { 1 } }; // hash1 < hash2
		auto timestampedHash1 = TimestampedHash(Timestamp(123), hash1);
		auto timestampedHash2 = TimestampedHash(Timestamp(123), hash1);
		auto timestampedHash3 = TimestampedHash(Timestamp(234), hash1);
		auto timestampedHash4 = TimestampedHash(Timestamp(123), hash2);
		auto timestampedHash5 = TimestampedHash(Timestamp(234), hash2);

		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(
				{ timestampedHash1, timestampedHash2 },
				{ timestampedHash3, timestampedHash4, timestampedHash5 });
	}

	// endregion

	// region serialize key

	TEST(TEST_CLASS, CanSerializeTimestampedHash) {
		// Arrange:
		auto key = TimestampedHash(test::GenerateRandomValue<Timestamp>(), test::GenerateRandomArray<Cached_Hash_Size>());

		// Act:
		auto result = SerializeKey(key);

		// Assert:
		ASSERT_EQ(sizeof(TimestampedHash), result.Size);
		EXPECT_EQ(test::AsBytePointer(&key), result.pData);
	}

	// endregion

	// region insertion operator

	TEST(TEST_CLASS, CanOutputToStream) {
		// Arrange:
		Hash256 hash;
		std::string hashString = "1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751";
		utils::ParseHexStringIntoContainer(hashString.c_str(), hashString.size(), hash);

		auto timestampedHash = TimestampedHash(Timestamp(98126), hash);

		// Act:
		auto str = test::ToString(timestampedHash);

		// Assert:
		EXPECT_EQ("1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751 @ 98126", str);
	}

	// endregion
}}
