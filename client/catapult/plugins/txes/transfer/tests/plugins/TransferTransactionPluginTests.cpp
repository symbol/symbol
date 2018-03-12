#include "src/plugins/TransferTransactionPlugin.h"
#include "src/model/TransferNotifications.h"
#include "src/model/TransferTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS TransferTransactionPluginTests

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(Transfer);

		template<typename TTraits>
		auto CreateTransactionWithMosaics(uint8_t numMosaics, uint16_t messageSize = 0) {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + numMosaics * sizeof(Mosaic) + messageSize;
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			pTransaction->Size = entitySize;
			pTransaction->MessageSize = messageSize;
			pTransaction->MosaicsCount = numMosaics;
			test::FillWithRandomData(pTransaction->Signer);
			test::FillWithRandomData(pTransaction->Recipient);
			return pTransaction;
		}
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Entity_Type_Transfer)

	PLUGIN_TEST(CanCalculateSize) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		transaction.Size = 0;
		transaction.MessageSize = 100;
		transaction.MosaicsCount = 7;

		// Act:
		auto realSize = pPlugin->calculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 100 + 7 * sizeof(Mosaic), realSize);
	}

	PLUGIN_TEST(CanExtractAccounts) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction{};
		test::FillWithRandomData(transaction.Signer);
		test::FillWithRandomData(transaction.Recipient);

		// Act:
		pPlugin->publish(transaction, sub);

		// Assert:
		EXPECT_EQ(1u, sub.numNotifications());
		EXPECT_EQ(1u, sub.numAddresses());
		EXPECT_EQ(0u, sub.numKeys());

		EXPECT_TRUE(sub.contains(transaction.Recipient));
	}

	// region balance change

	PLUGIN_TEST(CanExtractZeroTransfers) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithMosaics<TTraits>(0);

		// Act:
		pPlugin->publish(*pTransaction, sub);

		// Assert:
		EXPECT_EQ(1u, sub.numNotifications());
		EXPECT_EQ(0u, sub.numTransfers());
	}

	PLUGIN_TEST(CanExtractSingleTransfer) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		mocks::MockTypedNotificationSubscriber<TransferMosaicsNotification> mosaicsSub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithMosaics<TTraits>(1);
		*pTransaction->MosaicsPtr() = { MosaicId(123), Amount(9876) };

		// Act:
		pPlugin->publish(*pTransaction, sub);
		pPlugin->publish(*pTransaction, mosaicsSub);

		// Assert:
		EXPECT_EQ(3u, sub.numNotifications());
		EXPECT_EQ(1u, sub.numTransfers());
		EXPECT_TRUE(sub.contains(pTransaction->Signer, pTransaction->Recipient, MosaicId(123), Amount(9876)));

		ASSERT_EQ(1u, mosaicsSub.numMatchingNotifications());
		EXPECT_EQ(1u, mosaicsSub.matchingNotifications()[0].MosaicsCount);
		EXPECT_EQ(pTransaction->MosaicsPtr(), mosaicsSub.matchingNotifications()[0].MosaicsPtr);
	}

	PLUGIN_TEST(CanExtractMultipleTransfers) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		mocks::MockTypedNotificationSubscriber<TransferMosaicsNotification> mosaicsSub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithMosaics<TTraits>(3);
		auto pMosaic = pTransaction->MosaicsPtr();
		*pMosaic++ = { MosaicId(123), Amount(9876) };
		*pMosaic++ = { MosaicId(777), Amount(444) };
		*pMosaic++ = { MosaicId(625), Amount(25) };

		// Act:
		pPlugin->publish(*pTransaction, sub);
		pPlugin->publish(*pTransaction, mosaicsSub);

		// Assert:
		EXPECT_EQ(5u, sub.numNotifications());
		EXPECT_EQ(3u, sub.numTransfers());
		EXPECT_TRUE(sub.contains(pTransaction->Signer, pTransaction->Recipient, MosaicId(123), Amount(9876)));
		EXPECT_TRUE(sub.contains(pTransaction->Signer, pTransaction->Recipient, MosaicId(777), Amount(444)));
		EXPECT_TRUE(sub.contains(pTransaction->Signer, pTransaction->Recipient, MosaicId(625), Amount(25)));

		ASSERT_EQ(1u, mosaicsSub.numMatchingNotifications());
		EXPECT_EQ(3u, mosaicsSub.matchingNotifications()[0].MosaicsCount);
		EXPECT_EQ(pTransaction->MosaicsPtr(), mosaicsSub.matchingNotifications()[0].MosaicsPtr);
	}

	// endregion

	PLUGIN_TEST(CanExtractMessage) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<TransferMessageNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithMosaics<TTraits>(0, 17);

		// Act:
		pPlugin->publish(*pTransaction, sub);

		// Assert:
		EXPECT_EQ(2u, sub.numNotifications());
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		EXPECT_EQ(17u, sub.matchingNotifications()[0].MessageSize);
	}
}}
