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

#include "catapult/disruptor/ConsumerEntry.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

#define TEST_CLASS ConsumerEntryTests

	TEST(TEST_CLASS, CanCreateAnEntry) {
		// Arrange:
		auto level = 123u;
		ConsumerEntry consumer(level);

		// Assert:
		EXPECT_EQ(123u, consumer.level());
		EXPECT_EQ(0u, consumer.position());
	}

	TEST(TEST_CLASS, CanAdvanceConsumerPosition) {
		// Arrange:
		auto level = 123u;
		ConsumerEntry consumer(level);

		// Act:
		for (auto i = 0; i < 10; ++i)
			consumer.advance();

		// Assert:
		EXPECT_EQ(123u, consumer.level());
		EXPECT_EQ(10u, consumer.position());
	}
}}
