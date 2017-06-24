#include "catapult/utils/HashSet.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

	TEST(HashSetTests, Hash256PointerHasher_ReturnsSameHashOnlyWhenPointedToHashesAreEqual) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto copyHash = hash;
		auto differentHash = test::GenerateRandomData<Hash256_Size>();
		Hash256PointerHasher hasher;

		// Act + Assert:
		EXPECT_EQ(hasher(&hash), hasher(&hash));
		EXPECT_EQ(hasher(&hash), hasher(&copyHash));
		EXPECT_NE(hasher(&hash), hasher(&differentHash));
	}

	TEST(HashSetTests, Hash256PointerEquality_ReturnsTrueOnlyWhenPointedToHashesAreEqual) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto copyHash = hash;
		auto differentHash = test::GenerateRandomData<Hash256_Size>();
		Hash256PointerEquality equality;

		// Act + Assert:
		EXPECT_TRUE(equality(&hash, &hash));
		EXPECT_TRUE(equality(&hash, &copyHash));
		EXPECT_TRUE(equality(&copyHash, &hash));
		EXPECT_FALSE(equality(&hash, &differentHash));
		EXPECT_FALSE(equality(&differentHash, &hash));
	}
}}
