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

#include "catapult/crypto/SecureZero.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS SecureZeroTests

	TEST(TEST_CLASS, SecureZeroZerosOutBackingMemoryOfData) {
		// Arrange:
		auto buffer = test::GenerateRandomArray<625>();
		uint8_t* pRawData = buffer.data();

		// Sanity:
		std::array<uint8_t, 625> zeroData;
		std::fill(zeroData.begin(), zeroData.end(), static_cast<uint8_t>(0));
		EXPECT_FALSE(std::equal(zeroData.cbegin(), zeroData.cend(), pRawData, pRawData + buffer.size()));

		// Act:
		SecureZero(pRawData, 625);

		// Assert:
		EXPECT_TRUE(std::equal(zeroData.cbegin(), zeroData.cend(), pRawData, pRawData + buffer.size()));
		EXPECT_EQ(zeroData, buffer);
	}

	TEST(TEST_CLASS, SecureZeroZerosOutBackingMemoryOfArray) {
		// Arrange:
		uint64_t buffer[123];
		test::FillWithRandomData({ reinterpret_cast<uint8_t*>(buffer), sizeof(uint64_t) * 123 });

		// Sanity:
		uint64_t zeroData[123]{};
		auto equalMemory = 0 == std::memcmp(zeroData, buffer, sizeof(uint64_t) * 123);
		EXPECT_FALSE(equalMemory);

		// Act:
		SecureZero(buffer);

		// Assert:
		EXPECT_EQ_MEMORY(zeroData, buffer, sizeof(uint64_t) * 123);
	}

	TEST(TEST_CLASS, SecureZeroZerosOutBackingMemoryOfByteArray) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
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
}}
