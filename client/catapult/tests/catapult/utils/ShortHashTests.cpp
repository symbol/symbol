#include "catapult/utils/ShortHash.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS ShortHashTests

	// region hasher

	TEST(TEST_CLASS, HasherIsDeterministic) {
		// Arrange:
		ShortHashHasher hasher;
		auto data = ShortHash(123456789);

		// Act:
		auto result1 = hasher(data);
		auto result2 = hasher(data);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, HashHasSameValueAsShortHash) {
		// Arrange:
		ShortHashHasher hasher;
		auto data = ShortHash(123456789);

		// Act:
		auto result = hasher(data);

		// Assert:
		EXPECT_EQ(123456789u, result);
	}

	TEST(TEST_CLASS, EqualShortHashesHaveSameHash) {
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

	TEST(TEST_CLASS, DifferentShortHashesHaveDifferentHashes) {
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

	// endregion

	// region ToShortHash

	TEST(TEST_CLASS, CanConvertHashToShortHash) {
		// Arrange:
		Hash256 hash;
		ParseHexStringIntoContainer("031729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31C", 2 * Hash256_Size, hash);

		// Act:
		auto shortHash = ToShortHash(hash);

		// Assert:
		EXPECT_EQ(ShortHash(0xD1291703), shortHash);
	}

	// endregion
}}
