#include "addressextraction/src/AddressExtractionUtSubscriber.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/mocks/MockNotificationPublisher.h"
#include "tests/TestHarness.h"

namespace catapult { namespace addressextraction {

#define TEST_CLASS AddressExtractionUtSubscriberTests

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
		auto pSubscriber = CreateAddressExtractionChangeSubscriber(std::move(pNotificationPublisher));

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
		auto pSubscriber = CreateAddressExtractionChangeSubscriber(std::move(pNotificationPublisher));

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
		auto pSubscriber = CreateAddressExtractionChangeSubscriber(std::move(pNotificationPublisher));

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
		auto pSubscriber = CreateAddressExtractionChangeSubscriber(std::move(pNotificationPublisher));

		// Act:
		pSubscriber->flush();

		// Assert:
		EXPECT_EQ(0u, publisher.numPublishCalls());
	}
}}
