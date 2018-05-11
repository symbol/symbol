/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

using TimeSpan = catapult::test::DeterministicTimeSpan;

namespace catapult { namespace utils {

#define TEST_CLASS NetworkTimeTests

	TEST(TEST_CLASS, NetworkTimeReturnsExpectedTimestamp) {
		// Arrange:
		Timestamp networkTime;

		// Act:
		TimeSpan duration = test::RunDeterministicOperation([&networkTime]() {
			networkTime = NetworkTime();
		});

		// Assert:
		EXPECT_EQ(static_cast<uint64_t>(duration.count() - Epoch_Time.count()), networkTime.unwrap());
	}

	TEST(TEST_CLASS, ToNetworkTimeReturnsExpectedTimestamp) {
		// Arrange:
		Timestamp ts1(Epoch_Time.count());
		Timestamp ts2(Epoch_Time.count() + 1);
		Timestamp ts3(Epoch_Time.count() + 123456);

		// Act:
		Timestamp networkTime1 = ToNetworkTime(ts1);
		Timestamp networkTime2 = ToNetworkTime(ts2);
		Timestamp networkTime3 = ToNetworkTime(ts3);

		// Assert:
		EXPECT_EQ(Timestamp(0), networkTime1);
		EXPECT_EQ(Timestamp(1), networkTime2);
		EXPECT_EQ(Timestamp(123456), networkTime3);
	}

	TEST(TEST_CLASS, ToNetworkTimeThrowsIfSuppliedTimestampIsBeforeEpochTime) {
		// Arrange:
		Timestamp ts1(Epoch_Time.count() - 1);
		Timestamp ts2(Epoch_Time.count() - 10);
		Timestamp ts3(Epoch_Time.count() - 1000);

		// Act + Assert:
		EXPECT_THROW(ToNetworkTime(ts1), catapult_invalid_argument);
		EXPECT_THROW(ToNetworkTime(ts2), catapult_invalid_argument);
		EXPECT_THROW(ToNetworkTime(ts3), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, ToUnixTimeReturnsExpectedTimestamp) {
		// Arrange:
		Timestamp ts(123456);

		// Act:
		Timestamp unixTime = ToUnixTime(ts);

		// Assert:
		EXPECT_EQ(Timestamp(Epoch_Time.count() + 123456), unixTime);
	}

	TEST(TEST_CLASS, ToUnixTimeDetectsOverflow) {
		// Arrange:
		Timestamp ts(std::numeric_limits<uint64_t>::max() - Epoch_Time.count() + 1);

		// Act + Assert:
		EXPECT_THROW(ToUnixTime(ts), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, ConvertingTimeForthAndBackResultsInAnEquivalentTimestamp) {
		// Arrange: represents a network time
		auto ts = test::GenerateRandomValue<Timestamp>();

		// Act: network time -> unix time -> network time
		Timestamp converted = ToNetworkTime(ToUnixTime(ts));

		// Assert:
		EXPECT_EQ(ts, converted);
	}
}}
