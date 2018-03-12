#include "catapult/utils/ArraySet.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS ArraySetTests

	TEST(TEST_CLASS, ArrayPointerHasher_ReturnsSameHashOnlyWhenPointedToHashesAreEqual) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto hashCopy = hash;
		auto differentHash = test::GenerateRandomData<Hash256_Size>();
		ArrayPointerHasher<Hash256> hasher;

		// Act + Assert:
		EXPECT_EQ(hasher(&hash), hasher(&hash));
		EXPECT_EQ(hasher(&hash), hasher(&hashCopy));
		EXPECT_NE(hasher(&hash), hasher(&differentHash));
	}

	TEST(TEST_CLASS, ArrayPointerEquality_ReturnsTrueOnlyWhenPointedToHashesAreEqual) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto hashCopy = hash;
		auto differentHash = test::GenerateRandomData<Hash256_Size>();
		ArrayPointerEquality<Hash256> equality;

		// Act + Assert:
		EXPECT_TRUE(equality(&hash, &hash));
		EXPECT_TRUE(equality(&hash, &hashCopy));
		EXPECT_TRUE(equality(&hashCopy, &hash));
		EXPECT_FALSE(equality(&hash, &differentHash));
		EXPECT_FALSE(equality(&differentHash, &hash));
	}
}}
