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

#include "zeromq/src/PublisherUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace zeromq {

#define TEST_CLASS PublisherUtilsTests

	TEST(TEST_CLASS, CanCreateTopic) {
		// Arrange:
		TransactionMarker marker = TransactionMarker(0x37);
		auto address = test::GenerateRandomUnresolvedAddress();

		// Act:
		auto topic = CreateTopic(marker, address);

		// Assert:
		ASSERT_EQ(Address::Size + 1, topic.size());
		EXPECT_EQ(marker, TransactionMarker(topic[0]));
		EXPECT_EQ_MEMORY(address.data(), topic.data() + 1, Address::Size);
	}
}}
