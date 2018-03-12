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

	TEST(TEST_CLASS, CanNotifyMultipleBlockHeaders) {
		// Arrange:
		MqSubscriberContext context;
		context.subscribe(BlockMarker::Block_Marker);
		std::vector<std::unique_ptr<model::Block>> blocks;
		for (auto i = 0u; i < 3; ++i)
			blocks.push_back(test::GenerateEmptyRandomBlock());

		// Act:
		for (const auto& pBlock : blocks)
			context.notifyBlock(test::BlockToBlockElement(*pBlock));

		for (auto i = 0u; i < blocks.size(); ++i) {
			// Assert:
			zmq::multipart_t message;
			test::ZmqReceive(message, context.zmqSocket());

			test::AssertBlockHeaderMessage(message, test::BlockToBlockElement(*blocks[i]));
		}

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
			auto pBlock = test::GenerateRandomBlockWithTransactions(transactions);
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
