#include "partialtransaction/src/chain/AggregateCosignersNotificationPublisher.h"
#include "plugins/txes/aggregate/src/model/AggregateNotifications.h"
#include "catapult/model/WeakCosignedTransactionInfo.h"
#include "partialtransaction/tests/test/AggregateTransactionTestUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace chain {

#define TEST_CLASS AggregateCosignersNotificationPublisherTests

	// region basic

	TEST(TEST_CLASS, NonAggregateTransactionIsNotSupported) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<AggregateEmbeddedTransactionNotification> sub;
		AggregateCosignersNotificationPublisher publisher;
		auto wrapper = test::CreateAggregateTransaction(2);

		auto cosignatures = test::GenerateRandomDataVector<Cosignature>(4);
		auto transactionInfo = WeakCosignedTransactionInfo(wrapper.pTransaction.get(), &cosignatures);

		// - change the transaction type
		DEFINE_TRANSACTION_TYPE(Aggregate, Non_Aggregate_Type, 0xFF);
		wrapper.pTransaction->Type = Entity_Type_Non_Aggregate_Type;

		// Act + Assert: transfer type is not supported
		EXPECT_THROW(publisher.publish(transactionInfo, sub), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, AggregateWithCosignaturesIsNotSupported) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<AggregateEmbeddedTransactionNotification> sub;
		AggregateCosignersNotificationPublisher publisher;
		auto wrapper = test::CreateAggregateTransaction(2);

		auto cosignatures = test::GenerateRandomDataVector<Cosignature>(4);
		auto transactionInfo = WeakCosignedTransactionInfo(wrapper.pTransaction.get(), &cosignatures);

		// - make the transaction look like it has a cosignature
		//   (since the transactions are not iterated before the check, only the first transaction needs to be valid)
		wrapper.pTransaction->PayloadSize -= sizeof(Cosignature);

		// Act + Assert: aggregate must not have any cosignatures
		EXPECT_THROW(publisher.publish(transactionInfo, sub), catapult_invalid_argument);
	}

	// endregion

	// region publish: aggregate embedded transaction

	namespace {
		void AssertCanRaiseEmbeddedTransactionNotifications(
				uint8_t numTransactions,
				uint8_t numCosignatures,
				model::EntityType transactionType) {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<AggregateEmbeddedTransactionNotification> sub;
			AggregateCosignersNotificationPublisher publisher;
			auto wrapper = test::CreateAggregateTransaction(numTransactions);
			wrapper.pTransaction->Type = transactionType;

			auto cosignatures = test::GenerateRandomDataVector<Cosignature>(numCosignatures);
			auto transactionInfo = WeakCosignedTransactionInfo(wrapper.pTransaction.get(), &cosignatures);

			// Act:
			publisher.publish(transactionInfo, sub);

			// Assert: the plugin raises an embedded transaction notification for each transaction
			ASSERT_EQ(numTransactions, sub.numMatchingNotifications());
			for (auto i = 0u; i < numTransactions; ++i) {
				std::ostringstream out;
				out << "transaction at " << i << " (" << transactionType << ")";
				auto message = out.str();
				const auto& notification = sub.matchingNotifications()[i];

				EXPECT_EQ(wrapper.pTransaction->Signer, notification.Signer) << message;
				EXPECT_EQ(*wrapper.SubTransactions[i], notification.Transaction) << message;
				EXPECT_EQ(numCosignatures, notification.CosignaturesCount);
				EXPECT_EQ(numTransactions > 0 ? cosignatures.data() : nullptr, notification.CosignaturesPtr);
			}
		}
	}

	TEST(TEST_CLASS, EmptyAggregateDoesNotRaiseEmbeddedTransactionNotifications) {
		// Assert:
		for (auto transactionType : { Entity_Type_Aggregate_Complete, Entity_Type_Aggregate_Bonded })
			AssertCanRaiseEmbeddedTransactionNotifications(0, 0, transactionType);
	}

	TEST(TEST_CLASS, CanRaiseEmbeddedTransactionNotificationsFromAggregate) {
		// Assert:
		for (auto transactionType : { Entity_Type_Aggregate_Complete, Entity_Type_Aggregate_Bonded })
			AssertCanRaiseEmbeddedTransactionNotifications(2, 3, transactionType);
	}

	// endregion

	// region publish: aggregate cosignatures

	TEST(TEST_CLASS, CanRaiseAggregateCosignaturesNotificationsFromEmptyAggregate) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<AggregateCosignaturesNotification> sub;
		AggregateCosignersNotificationPublisher publisher;
		auto wrapper = test::CreateAggregateTransaction(0);

		auto cosignatures = test::GenerateRandomDataVector<Cosignature>(0);
		auto transactionInfo = WeakCosignedTransactionInfo(wrapper.pTransaction.get(), &cosignatures);

		// Act:
		publisher.publish(transactionInfo, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(wrapper.pTransaction->Signer, notification.Signer);
		EXPECT_EQ(0u, notification.TransactionsCount);
		EXPECT_FALSE(!!notification.TransactionsPtr);
		EXPECT_EQ(0u, notification.CosignaturesCount);
		EXPECT_FALSE(!!notification.CosignaturesPtr);
	}

	TEST(TEST_CLASS, CanRaiseAggregateCosignaturesNotificationsFromAggregate) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<AggregateCosignaturesNotification> sub;
		AggregateCosignersNotificationPublisher publisher;
		auto wrapper = test::CreateAggregateTransaction(2);

		auto cosignatures = test::GenerateRandomDataVector<Cosignature>(3);
		auto transactionInfo = WeakCosignedTransactionInfo(wrapper.pTransaction.get(), &cosignatures);

		// Act:
		publisher.publish(transactionInfo, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(wrapper.pTransaction->Signer, notification.Signer);
		EXPECT_EQ(2u, notification.TransactionsCount);
		EXPECT_EQ(wrapper.pTransaction->TransactionsPtr(), notification.TransactionsPtr);
		EXPECT_EQ(3u, notification.CosignaturesCount);
		EXPECT_EQ(cosignatures.data(), notification.CosignaturesPtr);
	}

	// endregion
}}
