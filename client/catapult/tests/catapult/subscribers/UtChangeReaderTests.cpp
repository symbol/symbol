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

#include "catapult/subscribers/UtChangeReader.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/TransactionInfoSerializer.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/other/mocks/MockUtChangeSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS UtChangeReaderTests

	namespace {
		void Write(
				io::OutputStream& outputStream,
				UtChangeOperationType operationType,
				const std::vector<model::TransactionInfo>& transactionInfos) {
			io::Write8(outputStream, utils::to_underlying_type(operationType));
			io::Write32(outputStream, static_cast<uint32_t>(transactionInfos.size()));
			for (const auto& transactionInfo : transactionInfos)
				io::WriteTransactionInfo(transactionInfo, outputStream);
		}

		void RunCanReadUtChangeTest(
				UtChangeOperationType operationType,
				size_t numTransactionInfos,
				const consumer<const std::vector<model::TransactionInfo>&, const mocks::MockUtChangeSubscriber&>& assertSubscriber) {
			// Arrange:
			auto transactionInfos = test::CreateTransactionInfosWithOptionalAddresses(numTransactionInfos);

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);
			Write(stream, operationType, transactionInfos);
			stream.seek(0);

			mocks::MockUtChangeSubscriber subscriber;

			// Act:
			ReadNextUtChange(stream, subscriber);

			// Assert:
			assertSubscriber(transactionInfos, subscriber);
			EXPECT_EQ(0u, subscriber.flushInfos().size());
		}
	}

	TEST(TEST_CLASS, CanReadAddChange) {
		// Act:
		RunCanReadUtChangeTest(UtChangeOperationType::Add, 7, [](const auto& transactionInfos, const auto& subscriber) {
			// Assert:
			ASSERT_EQ(transactionInfos.size(), subscriber.addedInfos().size());
			EXPECT_EQ(0u, subscriber.removedInfos().size());
			test::AssertEquivalent(transactionInfos, subscriber.addedInfos(), "added");
		});
	}

	TEST(TEST_CLASS, CanReadRemoveChange) {
		// Act:
		RunCanReadUtChangeTest(UtChangeOperationType::Remove, 7, [](const auto& transactionInfos, const auto& subscriber) {
			// Assert:
			ASSERT_EQ(transactionInfos.size(), subscriber.removedInfos().size());
			EXPECT_EQ(0u, subscriber.addedInfos().size());
			test::AssertEquivalent(transactionInfos, subscriber.removedInfos(), "added");
		});
	}

	TEST(TEST_CLASS, CanReadZeroChanges) {
		// Act:
		RunCanReadUtChangeTest(UtChangeOperationType::Add, 0, [](const auto&, const auto& subscriber) {
			// Assert:
			EXPECT_EQ(0u, subscriber.addedInfos().size());
			EXPECT_EQ(0u, subscriber.removedInfos().size());
		});
	}

	TEST(TEST_CLASS, ReadThrowsWhenChangeOperationTypeIsInvalid) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);
		io::Write8(stream, 0xFF);
		io::Write32(stream, 0);
		stream.seek(0);

		mocks::MockUtChangeSubscriber subscriber;

		// Act + Assert:
		EXPECT_THROW(ReadNextUtChange(stream, subscriber), catapult_invalid_argument);
	}
}}
