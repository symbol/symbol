#include "catapult/crypto/Hashes.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS HashesTests

	TEST(TEST_CLASS, EmptyStringHasExpectedRipemd160Hash) {
		// Act:
		auto data = test::ToVector("");
		Hash160 hash;
		Ripemd160(data, hash);

		// Assert:
		EXPECT_EQ("9C1185A5C5E9FC54612808977EE8F548B2258D31", test::ToHexString(hash));
	}

	TEST(TEST_CLASS, NonEmptyStringHasExpectedRipemd160Hash) {
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
		for (auto i = 0u; i < CountOf(dataSet); ++i) {
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

	namespace {
		// data taken from : https://homes.esat.kuleuven.be/~bosselae/ripemd160.html
		const char* Data_Set[] = {
			"",
			"a",
			"abc",
			"message digest",
			"abcdefghijklmnopqrstuvwxyz",
			"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
			"12345678901234567890123456789012345678901234567890123456789012345678901234567890"
		};
	}

	TEST(TEST_CLASS, SampleRipeMd160TestVectors) {
		// Arrange:
		std::string expectedHashes[] {
			"9C1185A5C5E9FC54612808977EE8F548B2258D31",
			"0BDC9D2D256B3EE9DAAE347BE6F4DC835A467FFE",
			"8EB208F7E05D987A9B044A8E98C6B087F15A0BFC",
			"5D0689EF49D2FAE572B881B123A85FFA21595F36",
			"F71C27109C692C1B56BBDCEB5B9D2865B3708DBC",
			"12A053384A9C0C88E405A06C27DCF49ADA62EB2B",
			"B0E20B6E3116640286ED3A87A5713079B21F5189",
			"9B752E45573D4B39F4DBD3323CAB82BF63326BFB"
		};

		ASSERT_EQ(CountOf(Data_Set), CountOf(expectedHashes));
		for (auto i = 0u; i < CountOf(Data_Set); ++i) {
			// Act:
			Hash160 hash;
			Ripemd160({ reinterpret_cast<const uint8_t*>(Data_Set[i]), strlen(Data_Set[i]) }, hash);

			// Assert:
			EXPECT_EQ(expectedHashes[i], test::ToHexString(hash));
		}
	}

	TEST(TEST_CLASS, MillionTimesAHasExpectedRipemd160Hash) {
		// Arrange:
		std::vector<uint8_t> data(1'000'000, 'a');
		std::string expectedHash = "52783243C1697BDBE16D37F97F68F08325DC1528";

		// Act:
		Hash160 hash;
		Ripemd160(data, hash);

		// Assert:
		EXPECT_EQ(expectedHash, test::ToHexString(hash));
	}
}}
