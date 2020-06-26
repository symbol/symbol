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

#include "zeromq/src/ZeroMqBlockChangeSubscriber.h"
#include "zeromq/src/PublisherUtils.h"
#include "zeromq/src/ZeroMqEntityPublisher.h"
#include "catapult/io/BlockChangeSubscriber.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/model/TransactionUtils.h"
#include "zeromq/tests/test/ZeroMqTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace zeromq {

#define TEST_CLASS ZeroMqBlockChangeSubscriberTests

	namespace {
		class MqSubscriberContext : public test::MqContextT<io::BlockChangeSubscriber> {
		public:
			MqSubscriberContext() : MqContextT(CreateZeroMqBlockChangeSubscriber)
			{}

		public:
			void notifyBlock(const model::BlockElement& blockElement) {
				subscriber().notifyBlock(blockElement);
			}

			void notifyDropBlocksAfter(Height height) {
				subscriber().notifyDropBlocksAfter(height);
			}
		};
	}

	// region basic tests

	TEST(TEST_CLASS, SubscriberDoesNotReceiveDataOnDifferentTopic) {
		// Arrange:
		uint64_t topic(0x12345678);
		MqSubscriberContext context;
		context.subscribe(topic);

		auto pBlock = test::GenerateEmptyRandomBlock();
		auto blockElement = test::BlockToBlockElement(*pBlock);

		// Act:
		context.notifyBlock(blockElement);

		// Assert:
		test::AssertNoPendingMessages(context.zmqSocket());
	}

	// endregion

	// region block header

	TEST(TEST_CLASS, CanNotifySingleBlockHeader) {
		// Arrange:
		MqSubscriberContext context;
		context.subscribe(BlockMarker::Block_Marker);

		auto pBlock = test::GenerateEmptyRandomBlock();
		auto blockElement = test::BlockToBlockElement(*pBlock);

		// Act:
		context.notifyBlock(blockElement);

		// Assert:
		zmq::multipart_t message;
		test::ZmqReceive(message, context.zmqSocket());

		test::AssertBlockHeaderMessage(message, blockElement);
		test::AssertNoPendingMessages(context.zmqSocket());
	}

	// endregion

	// region transactions

	namespace {
		std::vector<std::shared_ptr<model::Transaction>> CreateTransactions(const std::vector<std::pair<Key, Key>>& pairs) {
			std::vector<std::shared_ptr<model::Transaction>> transactions;
			for (const auto& pair : pairs)
				transactions.push_back(mocks::CreateMockTransactionWithSignerAndRecipient(pair.first, pair.second));

			return transactions;
		}

		void AssertMessages(MqSubscriberContext& context, const model::BlockElement& blockElement) {
			auto marker = TransactionMarker::Transaction_Marker;
			auto height = blockElement.Block.Height;
			for (const auto& transactionElement : blockElement.Transactions) {
				auto addresses = model::ExtractAddresses(transactionElement.Transaction, context.notificationPublisher());
				auto& zmqSocket = context.zmqSocket();
				test::AssertMessages(zmqSocket, marker, addresses, [&transactionElement, height](const auto& message, const auto& topic) {
					test::AssertTransactionElementMessage(message, topic, transactionElement, height);
				});
			}
		}

		void AssertCanNotifyBlockWithTransactions(const std::vector<Key>& keys, const std::vector<std::pair<Key, Key>>& pairs) {
			MqSubscriberContext context;
			auto transactions = CreateTransactions(pairs);
			auto pBlock = test::GenerateBlockWithTransactions(transactions);
			auto blockElement = test::BlockToBlockElement(*pBlock);

			// - subscribe to all relevant topics
			auto addresses = test::ToAddresses(keys);
			context.subscribeAll(TransactionMarker::Transaction_Marker, addresses);

			// Act:
			context.notifyBlock(blockElement);

			// Assert:
			AssertMessages(context, blockElement);
			test::AssertNoPendingMessages(context.zmqSocket());
		}
	}

	TEST(TEST_CLASS, CanNotifyBlockWithSingleTransaction) {
		// Arrange:
		auto keys = test::GenerateRandomDataVector<Key>(2);

		// Assert:
		AssertCanNotifyBlockWithTransactions(keys, { { keys[0], keys[1] } });
	}

	TEST(TEST_CLASS, CanNotifyBlockWithMultipleTransactions_DiffSigners_DiffRecipients) {
		// Arrange:
		auto keys = test::GenerateRandomDataVector<Key>(6);

		// Assert:
		AssertCanNotifyBlockWithTransactions(keys, {
			{ keys[0], keys[1] },
			{ keys[2], keys[3] },
			{ keys[4], keys[5] }
		});
	}

	TEST(TEST_CLASS, CanNotifyBlockWithMultipleTransactions_SameSigners_DiffRecipients) {
		// Arrange:
		auto keys = test::GenerateRandomDataVector<Key>(4);

		// Assert:
		AssertCanNotifyBlockWithTransactions(keys, {
			{ keys[1], keys[0] },
			{ keys[1], keys[2] },
			{ keys[1], keys[3] }
		});
	}

	TEST(TEST_CLASS, CanNotifyBlockWithMultipleTransactions_DiffSigners_SameRecipients) {
		// Arrange:
		auto keys = test::GenerateRandomDataVector<Key>(4);

		// Assert:
		AssertCanNotifyBlockWithTransactions(keys, {
			{ keys[0], keys[1] },
			{ keys[2], keys[1] },
			{ keys[3], keys[1] }
		});
	}

	TEST(TEST_CLASS, CanNotifyBlockWithMultipleTransactions_OneAccountOnly) {
		// Arrange:
		auto keys = test::GenerateRandomDataVector<Key>(1);

		// Assert:
		AssertCanNotifyBlockWithTransactions(keys, {
			{ keys[0], keys[0] },
			{ keys[0], keys[0] },
			{ keys[0], keys[0] }
		});
	}

	// endregion

	// region notifyDropBlocksAfter

	TEST(TEST_CLASS, CanNotifyDropBlocksAfter) {
		// Arrange:
		MqSubscriberContext context;
		context.subscribe(BlockMarker::Drop_Blocks_Marker);

		// Act:
		context.notifyDropBlocksAfter(Height(123));

		// Assert:
		zmq::multipart_t message;
		test::ZmqReceive(message, context.zmqSocket());

		test::AssertDropBlocksMessage(message, Height(123));
		test::AssertNoPendingMessages(context.zmqSocket());
	}

	// endregion
}}
