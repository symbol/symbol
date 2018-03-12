#include "zeromq/src/ZeroMqTransactionStatusSubscriber.h"
#include "zeromq/src/PublisherUtils.h"
#include "zeromq/src/ZeroMqEntityPublisher.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/model/TransactionStatus.h"
#include "zeromq/tests/test/ZeroMqTestUtils.h"
#include "zeromq/tests/test/ZeroMqTransactionsChangeTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace zeromq {

#define TEST_CLASS ZeroMqTransactionStatusSubscriberTests

	namespace {
		class MqSubscriberContext : public test::MqContextT<subscribers::TransactionStatusSubscriber> {
		public:
			using MqContext::subscribeAll;

		public:
			MqSubscriberContext() : MqContextT(CreateZeroMqTransactionStatusSubscriber)
			{}

		public:
			void notifyStatus(const model::Transaction& transaction, const Hash256& hash, uint32_t status) {
				subscriber().notifyStatus(transaction, hash, status);
			}

			void flush() {
				subscriber().flush();
			}

		public:
			void subscribeAll(TransactionMarker topicMarker, const std::vector<model::TransactionInfo>& transactionInfos) {
				for (const auto& transactionInfo : transactionInfos) {
					auto addresses = test::ExtractAddresses(test::ToMockTransaction(*transactionInfo.pEntity));
					test::SubscribeForAddresses(zmqSocket(), topicMarker, addresses);
				}

				waitForReceiveSuccess();
			}
		};
	}

	// region basic tests

	TEST(TEST_CLASS, SubscriberDoesNotReceiveDataOnDifferentTopic) {
		// Arrange:
		uint8_t topic(0x12);
		MqSubscriberContext context;
		context.subscribe(topic);
		auto pTransaction = test::GenerateTransactionWithDeadline(Timestamp(123));

		// Act:
		context.notifyStatus(*pTransaction, test::GenerateRandomData<Hash256_Size>(), 123);

		// Assert:
		test::AssertNoPendingMessages(context.zmqSocket());
	}

	// endregion

	// region notifyStatus

	namespace {
		constexpr size_t Num_Transactions = 5;
		constexpr TransactionMarker Marker = TransactionMarker::Transaction_Status_Marker;

		std::vector<model::TransactionInfo> CreateTransactionInfos(size_t count) {
			std::vector<model::TransactionInfo> transactionInfos;
			for (auto i = 0u; i < count; ++i)
				transactionInfos.push_back(test::CreateTransactionInfoWithDeadline(i * i));

			return transactionInfos;
		}
	}

	TEST(TEST_CLASS, CanAddSingleTransactionStatus) {
		// Arrange:
		MqSubscriberContext context;
		auto transactionInfos = CreateTransactionInfos(1);
		auto addresses = test::ExtractAddresses(test::ToMockTransaction(*transactionInfos[0].pEntity));
		context.subscribeAll(Marker, addresses);

		// Act:
		context.notifyStatus(*transactionInfos[0].pEntity, transactionInfos[0].EntityHash, 123);

		// Assert:
		model::TransactionStatus transactionStatus(transactionInfos[0].EntityHash, 123, transactionInfos[0].pEntity->Deadline);
		test::AssertMessages(context.zmqSocket(), Marker, addresses, [&transactionStatus](const auto& message, const auto& topic) {
			test::AssertTransactionStatusMessage(message, topic, transactionStatus);
		});

		test::AssertNoPendingMessages(context.zmqSocket());
	}

	TEST(TEST_CLASS, CanAddMultipleTransactionStatuses) {
		// Arrange:
		MqSubscriberContext context;
		auto transactionInfos = CreateTransactionInfos(Num_Transactions);
		context.subscribeAll(Marker, transactionInfos);

		// Act:
		auto i = 0u;
		for (const auto& transactionInfo : transactionInfos)
			context.notifyStatus(*transactionInfo.pEntity, transactionInfo.EntityHash, i++);

		// Assert:
		i = 0u;
		for (const auto& transactionInfo : transactionInfos) {
			model::TransactionStatus transactionStatus(transactionInfo.EntityHash, i++, transactionInfo.pEntity->Deadline);
			auto addresses = test::ExtractAddresses(test::ToMockTransaction(*transactionInfo.pEntity));
			test::AssertMessages(context.zmqSocket(), Marker, addresses, [&transactionStatus](const auto& message, const auto& topic) {
				test::AssertTransactionStatusMessage(message, topic, transactionStatus);
			});
		}

		test::AssertNoPendingMessages(context.zmqSocket());
	}

	// endregion

	// region flush

	TEST(TEST_CLASS, FlushDoesNotSendMessages) {
		// Assert:
		test::AssertFlushDoesNotSendMessages<MqSubscriberContext>();
	}

	// endregion
}}
