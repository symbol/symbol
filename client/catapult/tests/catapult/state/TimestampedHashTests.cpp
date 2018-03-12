#include "catapult/state/TimestampedHash.h"
#include "catapult/utils/HexParser.h"
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

		void AssertLessThanOperatorForDifferentValues(const TimestampedHash& lhs, const TimestampedHash& rhs) {
			EXPECT_TRUE(lhs < rhs);
			EXPECT_FALSE(rhs < lhs);
		}

		void AssertLessThanOperatorForEqualValues(const TimestampedHash& lhs, const TimestampedHash& rhs) {
			EXPECT_FALSE(lhs < rhs);
			EXPECT_FALSE(rhs < lhs);
		}
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateTimestampedHash) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto data = TimestampedHash(Timestamp(123), hash);

		// Assert:
		EXPECT_EQ(Timestamp(123), data.Time);
		EXPECT_EQ(GetPartialHash(hash), data.Hash);
	}

	TEST(TEST_CLASS, CanCreateTimestampedHashFromTimestamp) {
		// Arrange:
		auto data = TimestampedHash(Timestamp(123));

		// Assert:
		EXPECT_EQ(Timestamp(123), data.Time);
		EXPECT_EQ(TimestampedHash::HashType(), data.Hash);
	}

	TEST(TEST_CLASS, CanCreateDefaultTimestampedHash) {
		// Arrange:
		auto data = TimestampedHash();

		// Assert:
		EXPECT_EQ(Timestamp(0), data.Time);
		EXPECT_EQ(TimestampedHash::HashType(), data.Hash);
	}

	// endregion

	// region comparison operators

	TEST(TEST_CLASS, OperatorLessThanReturnsTrueForSmallerValuesAndFalseOtherwise) {
		// Arrange:
		Hash256 hash1{ {} };
		Hash256 hash2{ { 1 } }; // hash1 < hash2
		auto data1 = TimestampedHash(Timestamp(123), hash1);
		auto data2 = TimestampedHash(Timestamp(123), hash1);
		auto data3 = TimestampedHash(Timestamp(234), hash1);
		auto data4 = TimestampedHash(Timestamp(123), hash2);
		auto data5 = TimestampedHash(Timestamp(234), hash2);

		// Assert:
		AssertLessThanOperatorForEqualValues(data1, data2);
		AssertLessThanOperatorForDifferentValues(data1, data3);
		AssertLessThanOperatorForDifferentValues(data1, data4);
		AssertLessThanOperatorForDifferentValues(data1, data5);
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueIfAndOnlyIfTimestampAndHashAreEqual) {
		// Arrange:
		Hash256 hash1{ {} };
		Hash256 hash2{ { 1 } }; // hash1 < hash2
		auto data1 = TimestampedHash(Timestamp(123), hash1);
		auto data2 = TimestampedHash(Timestamp(123), hash1);
		auto data3 = TimestampedHash(Timestamp(234), hash1);
		auto data4 = TimestampedHash(Timestamp(123), hash2);
		auto data5 = TimestampedHash(Timestamp(234), hash2);

		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects({ data1, data2 }, { data3, data4, data5 });
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueIfAndOnlyIfTimestampAndHashAreNotEqual) {
		// Arrange:
		Hash256 hash1{ {} };
		Hash256 hash2{ { 1 } }; // hash1 < hash2
		auto data1 = TimestampedHash(Timestamp(123), hash1);
		auto data2 = TimestampedHash(Timestamp(123), hash1);
		auto data3 = TimestampedHash(Timestamp(234), hash1);
		auto data4 = TimestampedHash(Timestamp(123), hash2);
		auto data5 = TimestampedHash(Timestamp(234), hash2);

		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects({ data1, data2 }, { data3, data4, data5 });
	}

	// endregion

	// region insertion operator

	TEST(TEST_CLASS, CanOutputToStream) {
		// Arrange:
		Hash256 hash;
		std::string hashString = "1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751";
		utils::ParseHexStringIntoContainer(hashString.c_str(), hashString.size(), hash);

		auto data = TimestampedHash(Timestamp(98126), hash);

		// Act:
		auto str = test::ToString(data);

		// Assert:
		EXPECT_EQ("1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751 @ 98126", str);
	}

	// endregion
}}
