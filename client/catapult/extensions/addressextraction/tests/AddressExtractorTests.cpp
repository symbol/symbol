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
#include "tests/test/core/mocks/MockTransaction.h"
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

	namespace {
		std::vector<UnresolvedAddress> SeedTransactionsWithExtractedAddresses(
				model::BlockElement& blockElement,
				uint32_t numTransactions) {
			auto i = 0u;
			auto transactions = test::GenerateRandomTransactions(numTransactions);
			auto seedAddresses = test::GenerateRandomDataVector<UnresolvedAddress>(2 * numTransactions);
			for (const auto& pTransaction : transactions) {
				blockElement.Transactions.push_back(model::TransactionElement(*pTransaction));

				auto pAddressSet = std::make_shared<model::UnresolvedAddressSet>();
				pAddressSet->insert(seedAddresses[2 * i]);
				pAddressSet->insert(seedAddresses[2 * i + 1]);
				blockElement.Transactions.back().OptionalExtractedAddresses = std::move(pAddressSet);
				++i;
			}

			return seedAddresses;
		}

		void AddAddressResolutionStatement(
				model::BlockStatement& statement,
				const UnresolvedAddress& address,
				const std::vector<std::pair<model::ReceiptSource, Address>>& seeds) {
			model::AddressResolutionStatement resolutionStatement(address);
			for (const auto& seed : seeds)
				resolutionStatement.addResolution(seed.second, seed.first);

			statement.AddressResolutionStatements.emplace(address, resolutionStatement);
		}

		model::UnresolvedAddressSet ToAddressSet(
				const std::vector<UnresolvedAddress>& unresolvedAddresses,
				std::initializer_list<size_t> unresolvedAddressIndexes) {
			model::UnresolvedAddressSet result;

			for (auto index : unresolvedAddressIndexes)
				result.insert(unresolvedAddresses[index]);

			return result;
		}

		model::UnresolvedAddressSet ToAddressSet(
				const std::vector<UnresolvedAddress>& unresolvedAddresses,
				std::initializer_list<size_t> unresolvedAddressIndexes,
				const std::vector<Address>& addresses,
				std::initializer_list<size_t> addressIndexes) {
			auto result = ToAddressSet(unresolvedAddresses, unresolvedAddressIndexes);

			for (auto index : addressIndexes)
				result.insert(addresses[index].copyTo<UnresolvedAddress>());

			return result;
		}
	}

	TEST(TEST_CLASS, ExtractDoesNotAddTransactionResolvedAddressesWhenBlockStatementIsNotPresent_BlockElement) {
		// Arrange:
		TestContext context;

		// - create four transaction elements and associate two addresses with each transaction
		model::Block block;
		model::BlockElement blockElement(block);
		auto seedAddresses = SeedTransactionsWithExtractedAddresses(blockElement, 4);

		// Act:
		context.extractor().extract(blockElement);

		// Assert:
		EXPECT_EQ(0u, context.publisher().numPublishCalls());

		// - all have all expected addresses
		EXPECT_EQ(ToAddressSet(seedAddresses, { 0, 1 }), *blockElement.Transactions[0].OptionalExtractedAddresses);
		EXPECT_EQ(ToAddressSet(seedAddresses, { 2, 3 }), *blockElement.Transactions[1].OptionalExtractedAddresses);
		EXPECT_EQ(ToAddressSet(seedAddresses, { 4, 5 }), *blockElement.Transactions[2].OptionalExtractedAddresses);
		EXPECT_EQ(ToAddressSet(seedAddresses, { 6, 7 }), *blockElement.Transactions[3].OptionalExtractedAddresses);
	}

	TEST(TEST_CLASS, ExtractAddsTransactionResolvedAddressesWhenBlockStatementIsPresent_BlockElement) {
		// Arrange:
		TestContext context;

		// - create four transaction elements and associate two addresses with each transaction
		model::Block block;
		model::BlockElement blockElement(block);
		auto seedAddresses = SeedTransactionsWithExtractedAddresses(blockElement, 4);

		// - add some address resolution statements
		auto seedResolvedAddresses = test::GenerateRandomDataVector<Address>(8);
		auto pBlockStatement = std::make_shared<model::BlockStatement>();
		AddAddressResolutionStatement(*pBlockStatement, seedAddresses[2], {
			{ { 1, 0 }, seedResolvedAddresses[0] },
			{ { 2, 0 }, seedResolvedAddresses[1] },
			{ { 2, 9 }, seedResolvedAddresses[2] },
			{ { 3, 0 }, seedResolvedAddresses[3] }
		});
		AddAddressResolutionStatement(*pBlockStatement, seedAddresses[6], {
			{ { 3, 0 }, seedResolvedAddresses[4] },
			{ { 3, 9 }, seedResolvedAddresses[5] },
			{ { 4, 9 }, seedResolvedAddresses[6] },
			{ { 5, 0 }, seedResolvedAddresses[7] }
		});
		blockElement.OptionalStatement = std::move(pBlockStatement);

		// Act:
		context.extractor().extract(blockElement);

		// Assert:
		EXPECT_EQ(0u, context.publisher().numPublishCalls());

		// - all have all expected addresses
		EXPECT_EQ(ToAddressSet(seedAddresses, { 0, 1 }), *blockElement.Transactions[0].OptionalExtractedAddresses);
		EXPECT_EQ(
				ToAddressSet(seedAddresses, { 2, 3 }, seedResolvedAddresses, { 1, 2 }),
				*blockElement.Transactions[1].OptionalExtractedAddresses);
		EXPECT_EQ(ToAddressSet(seedAddresses, { 4, 5 }), *blockElement.Transactions[2].OptionalExtractedAddresses);
		EXPECT_EQ(
				ToAddressSet(seedAddresses, { 6, 7 }, seedResolvedAddresses, { 6 }),
				*blockElement.Transactions[3].OptionalExtractedAddresses);
	}

	// endregion
}}
