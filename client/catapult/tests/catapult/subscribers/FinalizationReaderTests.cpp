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

#include "catapult/subscribers/FinalizationReader.h"
#include "tests/test/core/FinalizationTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/other/mocks/MockFinalizationSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS FinalizationReaderTests

	namespace {
		void AssertEqual(
				const test::FinalizationNotification& expected,
				const mocks::FinalizationSubscriberFinalizedBlockParams& actual,
				const std::string& message) {
			EXPECT_EQ(expected.Round, actual.Round) << message;
			EXPECT_EQ(expected.Height, actual.Height) << message;
			EXPECT_EQ(expected.Hash, actual.Hash) << message;
		}
	}

	TEST(TEST_CLASS, CanReadSingle) {
		// Arrange:
		auto notification = test::GenerateRandomFinalizationNotification();

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);
		test::WriteFinalizationNotification(stream, notification);
		stream.seek(0);

		mocks::MockFinalizationSubscriber subscriber;

		// Act:
		ReadNextFinalization(stream, subscriber);

		// Assert:
		ASSERT_EQ(1u, subscriber.finalizedBlockParams().params().size());
		AssertEqual(notification, subscriber.finalizedBlockParams().params()[0], "at 0");
	}
}}
