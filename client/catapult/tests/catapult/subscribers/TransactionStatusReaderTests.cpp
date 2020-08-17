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

#include "catapult/subscribers/TransactionStatusReader.h"
#include "tests/test/core/TransactionStatusTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/other/mocks/MockTransactionStatusSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS TransactionStatusReaderTests

	namespace {
		// region test utils

		void AssertEqual(
				const test::TransactionStatusNotification& expected,
				const mocks::TransactionStatusSubscriberStatusParams& actual,
				const std::string& message) {
			EXPECT_EQ(*expected.pTransaction, *actual.pTransactionCopy) << message;
			EXPECT_EQ(expected.Hash, actual.HashCopy) << message;
			EXPECT_EQ(expected.Status, actual.Status) << message;
		}

		// endregion
	}

	TEST(TEST_CLASS, CanReadSingle) {
		// Arrange:
		auto notification = test::GenerateRandomTransactionStatusNotification(141);

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);
		test::WriteTransactionStatusNotification(stream, notification);
		stream.seek(0);

		mocks::MockTransactionStatusSubscriber subscriber;

		// Act:
		ReadNextTransactionStatus(stream, subscriber);

		// Assert:
		ASSERT_EQ(1u, subscriber.numNotifies());
		AssertEqual(notification, subscriber.params()[0], "at 0");

		EXPECT_EQ(0u, subscriber.numFlushes());
	}
}}
