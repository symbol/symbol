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

#include "catapult/subscribers/PtChangeReader.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/TransactionInfoSerializer.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/other/mocks/MockPtChangeSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS PtChangeReaderTests

	namespace {
		void Write(
				io::OutputStream& outputStream,
				PtChangeOperationType operationType,
				const std::vector<model::TransactionInfo>& transactionInfos) {
			io::Write8(outputStream, utils::to_underlying_type(operationType));
			io::Write32(outputStream, static_cast<uint32_t>(transactionInfos.size()));
			for (const auto& transactionInfo : transactionInfos)
				io::WriteTransactionInfo(transactionInfo, outputStream);
		}

		void RunCanReadPtChangeTest(
				PtChangeOperationType operationType,
				size_t numTransactionInfos,
				const consumer<const std::vector<model::TransactionInfo>&, const mocks::MockPtChangeSubscriber&>& assertSubscriber) {
			// Arrange:
			auto transactionInfos = test::CreateTransactionInfosWithOptionalAddresses(numTransactionInfos);

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);
			Write(stream, operationType, transactionInfos);
			stream.seek(0);

			mocks::MockPtChangeSubscriber subscriber;

			// Act:
			ReadNextPtChange(stream, subscriber);

			// Assert:
			assertSubscriber(transactionInfos, subscriber);
			EXPECT_EQ(0u, subscriber.flushInfos().size());
		}
	}

	TEST(TEST_CLASS, CanReadAddPartialsChange) {
		// Act:
		RunCanReadPtChangeTest(PtChangeOperationType::Add_Partials, 7, [](const auto& transactionInfos, const auto& subscriber) {
			// Assert:
			ASSERT_EQ(transactionInfos.size(), subscriber.addedInfos().size());
			EXPECT_EQ(0u, subscriber.removedInfos().size());
			EXPECT_EQ(0u, subscriber.addedCosignatureInfos().size());

			test::AssertEquivalent(transactionInfos, subscriber.addedInfos(), "added");
		});
	}

	TEST(TEST_CLASS, CanReadRemovePartialsChange) {
		// Act:
		RunCanReadPtChangeTest(PtChangeOperationType::Remove_Partials, 7, [](const auto& transactionInfos, const auto& subscriber) {
			// Assert:
			EXPECT_EQ(0u, subscriber.addedInfos().size());
			ASSERT_EQ(transactionInfos.size(), subscriber.removedInfos().size());
			EXPECT_EQ(0u, subscriber.addedCosignatureInfos().size());

			test::AssertEquivalent(transactionInfos, subscriber.removedInfos(), "removed");
		});
	}

	TEST(TEST_CLASS, CanReadZeroChanges) {
		// Act:
		RunCanReadPtChangeTest(PtChangeOperationType::Add_Partials, 0, [](const auto&, const auto& subscriber) {
			// Assert:
			EXPECT_EQ(0u, subscriber.addedInfos().size());
			EXPECT_EQ(0u, subscriber.removedInfos().size());
			EXPECT_EQ(0u, subscriber.addedCosignatureInfos().size());
		});
	}

	TEST(TEST_CLASS, CanReadCosignature) {
		// Arrange:
		auto cosignature = test::CreateRandomDetachedCosignature();
		auto transactionInfos = test::CreateTransactionInfosWithOptionalAddresses(1);

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);
		io::Write8(stream, utils::to_underlying_type(PtChangeOperationType::Add_Cosignature));
		stream.write({ reinterpret_cast<const uint8_t*>(&cosignature), sizeof(model::Cosignature) });
		io::WriteTransactionInfo(*transactionInfos.cbegin(), stream);
		stream.seek(0);

		mocks::MockPtChangeSubscriber subscriber;

		// Act:
		ReadNextPtChange(stream, subscriber);

		// Assert:
		EXPECT_EQ(0u, subscriber.addedInfos().size());
		EXPECT_EQ(0u, subscriber.removedInfos().size());
		ASSERT_EQ(1u, subscriber.addedCosignatureInfos().size());

		const auto& cosignatureInfo = subscriber.addedCosignatureInfos()[0];
		test::AssertCosignature(cosignature, cosignatureInfo.second);
		test::AssertEqual(*transactionInfos.cbegin(), *cosignatureInfo.first);

		EXPECT_EQ(0u, subscriber.flushInfos().size());
	}

	TEST(TEST_CLASS, ReadThrowsWhenChangeOperationTypeIsInvalid) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);
		io::Write8(stream, 0xFF);
		io::Write32(stream, 0);
		stream.seek(0);

		mocks::MockPtChangeSubscriber subscriber;

		// Act + Assert:
		EXPECT_THROW(ReadNextPtChange(stream, subscriber), catapult_invalid_argument);
	}
}}
