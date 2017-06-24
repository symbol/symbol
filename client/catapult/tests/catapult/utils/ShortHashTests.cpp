#include "catapult/utils/ShortHash.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

	TEST(ShortHashTests, HasherIsDeterministic) {
		// Arrange:
		ShortHashHasher hasher;
		auto data = ShortHash(123456789);

		// Act:
		auto result1 = hasher(data);
		auto result2 = hasher(data);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(ShortHashTests, HashHasSameValueAsShortHash) {
		// Arrange:
		ShortHashHasher hasher;
		auto data = ShortHash(123456789);

		// Act:
		auto result = hasher(data);

		// Assert:
		EXPECT_EQ(123456789u, result);
	}

	TEST(ShortHashTests, EqualShortHashesHaveSameHash) {
		// Arrange:
		ShortHashHasher hasher;
		auto data1 = ShortHash(123456789);
		auto data2 = ShortHash(123456789);

		// Act:
		auto result1 = hasher(data1);
		auto result2 = hasher(data2);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(ShortHashTests, DifferentShortHashesHaveDifferentHashes) {
		// Arrange:
		ShortHashHasher hasher;
		auto data1 = ShortHash(123456789);
		auto data2 = ShortHash(987654321);

		// Act:
		auto result1 = hasher(data1);
		auto result2 = hasher(data2);

		// Assert:
		EXPECT_NE(result1, result2);
	}
}}
