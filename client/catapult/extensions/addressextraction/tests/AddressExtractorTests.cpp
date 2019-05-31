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

#include "addressextraction/src/AddressExtractor.h"
#include "catapult/model/Elements.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockNotificationPublisher.h"
#include "tests/TestHarness.h"

namespace catapult { namespace addressextraction {

#define TEST_CLASS AddressExtractorTests

	namespace {
		// region TestContext

		class TestContext {
		public:
			TestContext()
					: m_pNotificationPublisher(std::make_unique<mocks::MockNotificationPublisher>())
					, m_notificationPublisher(*m_pNotificationPublisher)
					, m_extractor(std::move(m_pNotificationPublisher))
			{}

		public:
			const auto& publisher() const {
				return m_notificationPublisher;
			}

			const auto& extractor() const {
				return m_extractor;
			}

		private:
			std::unique_ptr<mocks::MockNotificationPublisher> m_pNotificationPublisher; // moved into m_extractor
			mocks::MockNotificationPublisher& m_notificationPublisher;
			AddressExtractor m_extractor;
		};

		// endregion
	}

	// region extract (TransactionInfo)

	TEST(TEST_CLASS, ExtractDelegatesToPublisherWhenExtractionRequired_TransactionInfo) {
		// Arrange:
		TestContext context;

		auto transactionInfo = test::CreateRandomTransactionInfo();
		transactionInfo.OptionalExtractedAddresses = nullptr;

		// Act:
		context.extractor().extract(transactionInfo);

		// Assert:
		EXPECT_EQ(1u, context.publisher().numPublishCalls());
		EXPECT_TRUE(!!transactionInfo.OptionalExtractedAddresses);
	}

	TEST(TEST_CLASS, ExtractBypassessPublisherWhenAlreadyExtracted_TransactionInfo) {
		// Arrange:
		TestContext context;
		auto pAddresses = std::make_shared<model::UnresolvedAddressSet>();

		auto transactionInfo = test::CreateRandomTransactionInfo();
		transactionInfo.OptionalExtractedAddresses = pAddresses;

		// Act:
		context.extractor().extract(transactionInfo);

		// Assert:
		EXPECT_EQ(0u, context.publisher().numPublishCalls());
		EXPECT_EQ(pAddresses, transactionInfo.OptionalExtractedAddresses);
	}

	// endregion

	// region extract (TransactionInfosSet)

	TEST(TEST_CLASS, ExtractDelegatesToPublisherOnlyWhenExtractionRequired_TransactionInfosSet) {
		// Arrange:
		TestContext context;
		auto pAddresses1 = std::make_shared<model::UnresolvedAddressSet>();
		auto pAddresses3 = std::make_shared<model::UnresolvedAddressSet>();

		// - create two infos with addresses and two without
		auto transactionInfos = test::CreateTransactionInfos(4);
		transactionInfos[0].OptionalExtractedAddresses = nullptr;
		transactionInfos[1].OptionalExtractedAddresses = pAddresses1;
		transactionInfos[2].OptionalExtractedAddresses = nullptr;
		transactionInfos[3].OptionalExtractedAddresses = pAddresses3;
		auto transactionInfoSet = test::CopyTransactionInfosToSet(transactionInfos);

		// Act:
		context.extractor().extract(transactionInfoSet);

		// Assert:
		EXPECT_EQ(2u, context.publisher().numPublishCalls());

		// - all have addresses
		for (auto& transactionInfo : transactionInfoSet)
			EXPECT_TRUE(!!transactionInfo.OptionalExtractedAddresses);

		// - the previously existing addresses are unchanged
		EXPECT_EQ(pAddresses1, transactionInfoSet.find(transactionInfos[1])->OptionalExtractedAddresses);
		EXPECT_EQ(pAddresses3, transactionInfoSet.find(transactionInfos[3])->OptionalExtractedAddresses);
	}

	// endregion

	// region extract (TransactionElement)

	TEST(TEST_CLASS, ExtractDelegatesToPublisherWhenExtractionRequired_TransactionElement) {
		// Arrange:
		TestContext context;

		auto pTransaction = test::GenerateRandomTransaction();
		auto transactionElement = model::TransactionElement(*pTransaction);
		transactionElement.OptionalExtractedAddresses = nullptr;

		// Act:
		context.extractor().extract(transactionElement);

		// Assert:
		EXPECT_EQ(1u, context.publisher().numPublishCalls());
		EXPECT_TRUE(!!transactionElement.OptionalExtractedAddresses);
	}

	TEST(TEST_CLASS, ExtractBypassessPublisherWhenAlreadyExtracted_TransactionElement) {
		// Arrange:
		TestContext context;
		auto pAddresses = std::make_shared<model::UnresolvedAddressSet>();

		auto pTransaction = test::GenerateRandomTransaction();
		auto transactionElement = model::TransactionElement(*pTransaction);
		transactionElement.OptionalExtractedAddresses = pAddresses;

		// Act:
		context.extractor().extract(transactionElement);

		// Assert:
		EXPECT_EQ(0u, context.publisher().numPublishCalls());
		EXPECT_EQ(pAddresses, transactionElement.OptionalExtractedAddresses);
	}

	// endregion

	// region extract (BlockElement)

	TEST(TEST_CLASS, ExtractDelegatesToPublisherOnlyWhenExtractionRequired_BlockElement) {
		// Arrange:
		TestContext context;
		auto pAddresses1 = std::make_shared<model::UnresolvedAddressSet>();
		auto pAddresses3 = std::make_shared<model::UnresolvedAddressSet>();

		// - create two elements with addresses and two without
		model::Block block;
		model::BlockElement blockElement(block);
		auto transactions = test::GenerateRandomTransactions(4);
		for (const auto& pTransaction : transactions)
			blockElement.Transactions.push_back(model::TransactionElement(*pTransaction));

		blockElement.Transactions[0].OptionalExtractedAddresses = nullptr;
		blockElement.Transactions[1].OptionalExtractedAddresses = pAddresses1;
		blockElement.Transactions[2].OptionalExtractedAddresses = nullptr;
		blockElement.Transactions[3].OptionalExtractedAddresses = pAddresses3;

		// Act:
		context.extractor().extract(blockElement);

		// Assert:
		EXPECT_EQ(2u, context.publisher().numPublishCalls());

		// - all have addresses
		for (auto& transactionElement : blockElement.Transactions)
			EXPECT_TRUE(!!transactionElement.OptionalExtractedAddresses);

		// - the previously existing addresses are unchanged
		EXPECT_EQ(pAddresses1, blockElement.Transactions[1].OptionalExtractedAddresses);
		EXPECT_EQ(pAddresses3, blockElement.Transactions[3].OptionalExtractedAddresses);
	}

	// endregion
}}
