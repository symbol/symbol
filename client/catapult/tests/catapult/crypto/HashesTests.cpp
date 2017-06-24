#include "catapult/crypto/Hashes.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {
	TEST(HashesTests, EmptyStringHasExpectedRipemd160Hash) {
		// Act:
		auto data = test::ToVector("");
		Hash160 hash;
		Ripemd160(data, hash);

		// Assert:
		EXPECT_EQ("9C1185A5C5E9FC54612808977EE8F548B2258D31", test::ToHexString(hash));
	}

	TEST(HashesTests, NonEmptyStringHasExpectedRipemd160Hash) {
		// Arrange:
		std::string dataSet[] {
			u8"The quick brown fox jumps over the lazy dog",
			u8"Kitten Kaboodle",
			u8"Lorem ipsum dolor sit amet",
			u8"GimreJaguar0625BloodyRookie",
			u8"The ripe taste of cheese improves with age",
		};
		std::string expectedHashes[] {
			"37F332F68DB77BD9D7EDD4969571AD671CF9DD3B",
			"114C70B78838555E6C3AB418F3052A949F73544A",
			"7D0982BE59EBE828D02AA0D031AA6651644D60DA",
			"5A4535208909435DECD5C7D6D818F67626A177E4",
			"1B3ACB0409F7BA78A0BE07A2DE5454DCB0D48817",
		};

		ASSERT_EQ(CountOf(dataSet), CountOf(expectedHashes));
		for (size_t i = 0; i < CountOf(dataSet); ++i) {
			// Arrange:
			auto hex = test::ToHexString(reinterpret_cast<const uint8_t*>(dataSet[i].c_str()), dataSet[i].size());
			auto data = test::ToVector(hex);

			// Act:
			Hash160 hash;
			Ripemd160(data, hash);

			// Assert:
			EXPECT_EQ(expectedHashes[i], test::ToHexString(hash));
		}
	}
}}
