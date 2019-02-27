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

#include "addressextraction/src/AddressExtractionUtChangeSubscriber.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/mocks/MockNotificationPublisher.h"
#include "tests/TestHarness.h"

namespace catapult { namespace addressextraction {

#define TEST_CLASS AddressExtractionUtChangeSubscriberTests

	namespace {
		cache::UtChangeSubscriber::TransactionInfos MoveToSetAndRemoveExtractedAddresses(
				std::vector<model::TransactionInfo>&& transactionInfos) {
			for (auto& transactionInfo : transactionInfos)
				transactionInfo.OptionalExtractedAddresses = nullptr;

			return test::CopyTransactionInfosToSet(transactionInfos);
		}
	}

	TEST(TEST_CLASS, NotifyAddsExtractsTransactionAddresses) {
		// Arrange:
		auto pNotificationPublisher = std::make_unique<mocks::MockNotificationPublisher>();
		const auto& publisher = *pNotificationPublisher;
		auto pSubscriber = CreateAddressExtractionUtChangeSubscriber(std::move(pNotificationPublisher));

		auto transactionInfoSet = MoveToSetAndRemoveExtractedAddresses(test::CreateTransactionInfos(3));

		// Act:
		pSubscriber->notifyAdds(transactionInfoSet);

		// Assert: publisher was called once for each info
		EXPECT_EQ(3u, publisher.numPublishCalls());

		// - all infos have addresses
		for (auto& transactionInfo : transactionInfoSet)
			EXPECT_TRUE(!!transactionInfo.OptionalExtractedAddresses);
	}

	TEST(TEST_CLASS, NotifyAddsExtractsTransactionAddressesOnlyWhenTransactionAddressesAreNotPresent) {
		// Arrange:
		auto pNotificationPublisher = std::make_unique<mocks::MockNotificationPublisher>();
		const auto& publisher = *pNotificationPublisher;
		auto pSubscriber = CreateAddressExtractionUtChangeSubscriber(std::move(pNotificationPublisher));

		// - create two infos with addresses and two without
		auto transactionInfos = test::CreateTransactionInfos(4);
		transactionInfos[1].OptionalExtractedAddresses = nullptr;
		transactionInfos[3].OptionalExtractedAddresses = nullptr;

		auto transactionInfoSet = test::CopyTransactionInfosToSet(transactionInfos);

		// Act:
		pSubscriber->notifyAdds(transactionInfoSet);

		// Assert: publisher was only called twice (once for each info without extracted addresses)
		EXPECT_EQ(2u, publisher.numPublishCalls());

		// - all infos have addresses
		for (auto& transactionInfo : transactionInfoSet)
			EXPECT_TRUE(!!transactionInfo.OptionalExtractedAddresses);

		// - the previously existing addresses are unchanged
		EXPECT_EQ(
				transactionInfos[0].OptionalExtractedAddresses.get(),
				transactionInfoSet.find(transactionInfos[0])->OptionalExtractedAddresses.get());
		EXPECT_EQ(
				transactionInfos[2].OptionalExtractedAddresses.get(),
				transactionInfoSet.find(transactionInfos[2])->OptionalExtractedAddresses.get());
	}

	TEST(TEST_CLASS, NotifyRemovesDoesNotExtractTransactionAddresses) {
		// Arrange:
		auto pNotificationPublisher = std::make_unique<mocks::MockNotificationPublisher>();
		const auto& publisher = *pNotificationPublisher;
		auto pSubscriber = CreateAddressExtractionUtChangeSubscriber(std::move(pNotificationPublisher));

		auto transactionInfoSet = MoveToSetAndRemoveExtractedAddresses(test::CreateTransactionInfos(3));

		// Act:
		pSubscriber->notifyRemoves(transactionInfoSet);

		// Assert: no addresses were extracted
		EXPECT_EQ(0u, publisher.numPublishCalls());

		for (auto& transactionInfo : transactionInfoSet)
			EXPECT_FALSE(!!transactionInfo.OptionalExtractedAddresses);
	}

	TEST(TEST_CLASS, FlushDoesNothing) {
		// Arrange:
		auto pNotificationPublisher = std::make_unique<mocks::MockNotificationPublisher>();
		const auto& publisher = *pNotificationPublisher;
		auto pSubscriber = CreateAddressExtractionUtChangeSubscriber(std::move(pNotificationPublisher));

		// Act:
		pSubscriber->flush();

		// Assert:
		EXPECT_EQ(0u, publisher.numPublishCalls());
	}
}}
