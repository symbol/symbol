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

#include "catapult/utils/NetworkTime.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS NetworkTimeTests

	TEST(TEST_CLASS, NetworkTimeReturnsExpectedTimestamp) {
		// Arrange:
		constexpr auto Epoch_Delta_Millis = 1459468800000ull;
		NetworkTime networkTime(utils::TimeSpan::FromSeconds(Epoch_Delta_Millis / 1000));

		Timestamp now;

		// Act:
		auto duration = test::RunDeterministicOperation([&networkTime, &now]() {
			now = networkTime.now();
		});

		// Assert:
		EXPECT_EQ(Timestamp(static_cast<uint64_t>(duration.count()) - Epoch_Delta_Millis), now);
	}

	TEST(TEST_CLASS, ToNetworkTimeReturnsExpectedTimestamp) {
		// Arrange:
		constexpr auto Epoch_Delta_Millis = 1559468800000ull;
		NetworkTime networkTime(utils::TimeSpan::FromSeconds(Epoch_Delta_Millis / 1000));

		Timestamp ts1(Epoch_Delta_Millis);
		Timestamp ts2(Epoch_Delta_Millis + 1);
		Timestamp ts3(Epoch_Delta_Millis + 123456);

		// Act:
		auto networkTime1 = networkTime.toNetworkTime(ts1);
		auto networkTime2 = networkTime.toNetworkTime(ts2);
		auto networkTime3 = networkTime.toNetworkTime(ts3);

		// Assert:
		EXPECT_EQ(Timestamp(0), networkTime1);
		EXPECT_EQ(Timestamp(1), networkTime2);
		EXPECT_EQ(Timestamp(123456), networkTime3);
	}

	TEST(TEST_CLASS, ToNetworkTimeThrowsWhenSuppliedTimestampIsBeforeEpochTime) {
		// Arrange:
		constexpr auto Epoch_Delta_Millis = 1559468800000ull;
		NetworkTime networkTime(utils::TimeSpan::FromSeconds(Epoch_Delta_Millis / 1000));

		Timestamp ts1(Epoch_Delta_Millis - 1);
		Timestamp ts2(Epoch_Delta_Millis - 10);
		Timestamp ts3(Epoch_Delta_Millis - 1000);

		// Act + Assert:
		EXPECT_THROW(networkTime.toNetworkTime(ts1), catapult_invalid_argument);
		EXPECT_THROW(networkTime.toNetworkTime(ts2), catapult_invalid_argument);
		EXPECT_THROW(networkTime.toNetworkTime(ts3), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, ToUnixTimeReturnsExpectedTimestamp) {
		// Arrange:
		constexpr auto Epoch_Delta_Millis = 1659468800000ull;
		NetworkTime networkTime(utils::TimeSpan::FromSeconds(Epoch_Delta_Millis / 1000));

		Timestamp ts(123456);

		// Act:
		auto unixTime = networkTime.toUnixTime(ts);

		// Assert:
		EXPECT_EQ(Timestamp(Epoch_Delta_Millis + 123456), unixTime);
	}

	TEST(TEST_CLASS, ToUnixTimeDetectsOverflow) {
		// Arrange:
		constexpr auto Epoch_Delta_Millis = 1659468800000ull;
		NetworkTime networkTime(utils::TimeSpan::FromSeconds(Epoch_Delta_Millis / 1000));

		Timestamp ts(std::numeric_limits<uint64_t>::max() - Epoch_Delta_Millis + 1);

		// Act + Assert:
		EXPECT_THROW(networkTime.toUnixTime(ts), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, ConvertingTimeForthAndBackResultsInAnEquivalentTimestamp) {
		// Arrange:
		constexpr auto Epoch_Delta_Millis = 1759468800000ul;
		NetworkTime networkTime(utils::TimeSpan::FromSeconds(Epoch_Delta_Millis / 1000));

		// -  represents a network time
		auto ts = test::GenerateRandomValue<Timestamp>();

		// Act: network time -> unix time -> network time
		auto converted = networkTime.toNetworkTime(networkTime.toUnixTime(ts));

		// Assert:
		EXPECT_EQ(ts, converted);
	}
}}
