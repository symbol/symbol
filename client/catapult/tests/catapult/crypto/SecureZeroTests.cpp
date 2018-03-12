#include "catapult/crypto/SecureZero.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS SecureZeroTests

	TEST(TEST_CLASS, SecureZeroZerosOutBackingMemoryOfKey) {
		// Arrange:
		auto key = test::GenerateRandomData<Key_Size>();
		const uint8_t* pRawKey = key.data();

		// Sanity:
		Key zeroKey;
		std::fill(zeroKey.begin(), zeroKey.end(), static_cast<uint8_t>(0));
		EXPECT_FALSE(std::equal(zeroKey.cbegin(), zeroKey.cend(), pRawKey, pRawKey + key.size()));

		// Act:
		SecureZero(key);

		// Assert:
		EXPECT_TRUE(std::equal(zeroKey.cbegin(), zeroKey.cend(), pRawKey, pRawKey + key.size()));
		EXPECT_EQ(zeroKey, key);
	}

	TEST(TEST_CLASS, SecureZeroZerosOutBackingMemoryOfData) {
		// Arrange:
		auto data = test::GenerateRandomData<625>();
		uint8_t* pRawData = data.data();

		// Sanity:
		std::array<uint8_t, 625> zeroData;
		std::fill(zeroData.begin(), zeroData.end(), static_cast<uint8_t>(0));
		EXPECT_FALSE(std::equal(zeroData.cbegin(), zeroData.cend(), pRawData, pRawData + data.size()));

		// Act:
		SecureZero(pRawData, 625);

		// Assert:
		EXPECT_TRUE(std::equal(zeroData.cbegin(), zeroData.cend(), pRawData, pRawData + data.size()));
		EXPECT_EQ(zeroData, data);
	}
}}
