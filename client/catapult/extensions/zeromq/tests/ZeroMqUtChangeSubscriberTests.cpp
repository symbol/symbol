#include "zeromq/src/ZeroMqUtChangeSubscriber.h"
#include "zeromq/src/PublisherUtils.h"
#include "catapult/model/Cosignature.h"
#include "zeromq/tests/test/ZeroMqTestUtils.h"
#include "zeromq/tests/test/ZeroMqTransactionsChangeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace zeromq {

#define TEST_CLASS ZeroMqUtChangeSubscriberTests

	namespace {
		class MqSubscriberContext : public test::MqContextT<cache::UtChangeSubscriber> {
		public:
			using MqContext::subscribeAll;

		public:
			MqSubscriberContext() : MqContextT(CreateZeroMqUtChangeSubscriber)
			{}

		public:
			void notifyAdd(const model::TransactionInfo& transactionInfo) {
				cache::UtChangeSubscriber::TransactionInfos transactionInfos;
				transactionInfos.emplace(transactionInfo.copy());
				notifyAdds(transactionInfos);
			}

			void notifyAdds(const cache::UtChangeSubscriber::TransactionInfos& transactionInfos) {
				subscriber().notifyAdds(transactionInfos);
			}

			void notifyRemove(const model::TransactionInfo& transactionInfo) {
				cache::UtChangeSubscriber::TransactionInfos transactionInfos;
				transactionInfos.emplace(transactionInfo.copy());
				notifyRemoves(transactionInfos);
			}

			void notifyRemoves(const cache::UtChangeSubscriber::TransactionInfos& transactionInfos) {
				subscriber().notifyRemoves(transactionInfos);
			}

			void flush() {
				subscriber().flush();
			}

		public:
			template<typename TTransactionInfos>
			void subscribeAll(TransactionMarker topicMarker, const TTransactionInfos& transactionInfos) {
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
		uint64_t topic(0x12345678);
		MqSubscriberContext context;
		context.subscribe(topic);
		auto transactionInfo = test::CreateRandomTransactionInfo();

		// Act:
		context.notifyAdd(transactionInfo);

		// Assert:
		test::AssertNoPendingMessages(context.zmqSocket());
	}

	// endregion

	namespace {
		constexpr size_t Num_Transactions = 5;
		constexpr auto Add_Marker = TransactionMarker::Unconfirmed_Transaction_Add_Marker;
		constexpr auto Remove_Marker = TransactionMarker::Unconfirmed_Transaction_Remove_Marker;
	}

	// region notifyAdd

	TEST(TEST_CLASS, CanAddSingleTransaction) {
		// Assert:
		test::AssertCanAddSingleTransaction<MqSubscriberContext>(Add_Marker, [](auto& context, const auto& transactionInfo) {
			context.notifyAdd(transactionInfo);
		});
	}

	TEST(TEST_CLASS, CanAddMultipleTransactions_SingleCall) {
		// Assert:
		test::AssertCanAddMultipleTransactions<MqSubscriberContext>(
				Add_Marker,
				Num_Transactions,
				[](auto& context, const auto& transactionInfos) {
					context.notifyAdds(transactionInfos);
				});
	}

	TEST(TEST_CLASS, CanAddMultipleTransactions_MultipleCalls) {
		// Assert:
		test::AssertCanAddMultipleTransactions<MqSubscriberContext>(
				Add_Marker,
				Num_Transactions,
				[](auto& context, const auto& transactionInfos) {
					for (const auto& transactionInfo : transactionInfos)
						context.notifyAdd(transactionInfo);
				});
	}

	// endregion

	// region notifyRemove

	TEST(TEST_CLASS, CanRemoveSingleTransaction) {
		// Assert:
		test::AssertCanRemoveSingleTransaction<MqSubscriberContext>(Remove_Marker, [](auto& context, const auto& transactionInfo) {
			context.notifyRemove(transactionInfo);
		});
	}

	TEST(TEST_CLASS, CanRemoveMultipleTransactions_SingleCall) {
		// Assert:
		test::AssertCanRemoveMultipleTransactions<MqSubscriberContext>(
				Remove_Marker,
				Num_Transactions,
				[](auto& context, const auto& transactionInfos) {
					context.notifyRemoves(transactionInfos);
				});
	}

	TEST(TEST_CLASS, CanRemoveMultipleTransactions_MultipleCalls) {
		// Assert:
		test::AssertCanRemoveMultipleTransactions<MqSubscriberContext>(
				Remove_Marker,
				Num_Transactions,
				[](auto& context, const auto& transactionInfos) {
					for (const auto& transactionInfo : transactionInfos)
						context.notifyRemove(transactionInfo);
				});
	}

	// endregion

	// region flush

	TEST(TEST_CLASS, FlushDoesNotSendMessages) {
		// Assert:
		test::AssertFlushDoesNotSendMessages<MqSubscriberContext>();
	}

	// endregion
}}
