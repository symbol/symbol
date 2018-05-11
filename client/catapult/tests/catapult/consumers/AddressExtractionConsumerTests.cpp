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

#include "catapult/consumers/BlockConsumers.h"
#include "catapult/consumers/TransactionConsumers.h"
#include "catapult/model/Address.h"
#include "tests/catapult/consumers/test/ConsumerTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace consumers {

#define BLOCK_TEST_CLASS BlockAddressExtractionConsumerTests
#define TRANSACTION_TEST_CLASS TransactionAddressExtractionConsumerTests

	namespace {
		void AssertExtractedAddress(const model::TransactionElement& transactionElement, size_t id) {
			auto message = "transaction at " + std::to_string(id);
			const auto& pAddresses = transactionElement.OptionalExtractedAddresses;
			ASSERT_TRUE(!!pAddresses) << message;
			EXPECT_EQ(1u, pAddresses->size()) << message;

			const auto& transaction = transactionElement.Transaction;
			auto senderAddress = model::PublicKeyToAddress(transaction.Signer, transaction.Network());
			EXPECT_TRUE(pAddresses->cend() != pAddresses->find(senderAddress)) << message;
		}
	}

	// region block

	namespace {
		void AssertBlockAddressesAreExtractedCorrectly(uint32_t numBlocks, uint32_t numTransactionsPerBlock) {
			// Arrange:
			std::vector<std::shared_ptr<const model::Block>> blocks;
			std::vector<const model::Block*> rawBlocks;
			for (auto i = 0u; i < numBlocks; ++i) {
				blocks.push_back(test::GenerateBlockWithTransactionsAtHeight(numTransactionsPerBlock, 10 + i));
				rawBlocks.push_back(blocks.back().get());
			}

			auto registry = mocks::CreateDefaultTransactionRegistry();
			auto pPublisher = model::CreateNotificationPublisher(registry, model::PublicationMode::Basic);
			auto input = test::CreateBlockElements(rawBlocks);

			// Act:
			auto result = CreateBlockAddressExtractionConsumer(*pPublisher)(input);

			// Assert:
			test::AssertContinued(result);

			auto i = 0u;
			for (const auto& element : input) {
				for (const auto& transactionElement : element.Transactions)
					AssertExtractedAddress(transactionElement, i++);
			}

			// Sanity:
			EXPECT_EQ(numBlocks * numTransactionsPerBlock, i);
		}
	}

	TEST(BLOCK_TEST_CLASS, CanProcessZeroEntities) {
		// Assert:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPublisher = model::CreateNotificationPublisher(registry, model::PublicationMode::Basic);
		test::AssertPassthroughForEmptyInput(CreateBlockAddressExtractionConsumer(*pPublisher));
	}

	TEST(BLOCK_TEST_CLASS, CanProcessSingleEntity) {
		// Assert:
		AssertBlockAddressesAreExtractedCorrectly(1, 0);
	}

	TEST(BLOCK_TEST_CLASS, CanProcessSingleEntityWithTransactions) {
		// Assert:
		AssertBlockAddressesAreExtractedCorrectly(1, 3);
	}

	TEST(BLOCK_TEST_CLASS, CanProcessMultipleEntities) {
		// Assert:
		AssertBlockAddressesAreExtractedCorrectly(3, 0);
	}

	TEST(BLOCK_TEST_CLASS, CanProcessMultipleEntitiesWithTransactions) {
		// Assert:
		AssertBlockAddressesAreExtractedCorrectly(3, 4);
	}

	// endregion

	// region transaction

	namespace {
		void AssertTransactionAddressesAreExtractedCorrectly(uint32_t numTransactions) {
			// Arrange:
			auto registry = mocks::CreateDefaultTransactionRegistry();
			auto pPublisher = model::CreateNotificationPublisher(registry, model::PublicationMode::Basic);
			auto input = test::CreateTransactionElements(numTransactions);

			// Act:
			auto result = CreateTransactionAddressExtractionConsumer(*pPublisher)(input);

			// Assert:
			test::AssertContinued(result);

			auto i = 0u;
			for (const auto& transactionElement : input)
				AssertExtractedAddress(transactionElement, i++);

			// Sanity:
			EXPECT_EQ(numTransactions, i);
		}
	}

	TEST(TRANSACTION_TEST_CLASS, CanProcessZeroEntities) {
		// Assert:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPublisher = model::CreateNotificationPublisher(registry, model::PublicationMode::Basic);
		test::AssertPassthroughForEmptyInput(CreateTransactionAddressExtractionConsumer(*pPublisher));
	}

	TEST(TRANSACTION_TEST_CLASS, CanProcessSingleEntity) {
		// Assert:
		AssertTransactionAddressesAreExtractedCorrectly(1);
	}

	TEST(TRANSACTION_TEST_CLASS, CanProcessMultipleEntities) {
		// Assert:
		AssertTransactionAddressesAreExtractedCorrectly(3);
	}

	// endregion
}}
