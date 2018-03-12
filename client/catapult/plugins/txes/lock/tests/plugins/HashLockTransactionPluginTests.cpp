#include "src/plugins/HashLockTransactionPlugin.h"
#include "src/model/HashLockTransaction.h"
#include "src/model/LockNotifications.h"
#include "tests/test/LockTransactionUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS HashLockTransactionPluginTests

	// region TransactionPlugin

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(HashLock);
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Entity_Type_Hash_Lock)

	PLUGIN_TEST(CanCalculateSize) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin();
		typename TTraits::TransactionType transaction;

		// Act:
		auto realSize = pPlugin->calculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(typename TTraits::TransactionType), realSize);
	}

	// endregion

	// region accounts extraction

	PLUGIN_TEST(CanExtractAccounts) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction{};

		// Act:
		pPlugin->publish(transaction, sub);

		// Assert:
		EXPECT_EQ(4u, sub.numNotifications());
		EXPECT_EQ(0u, sub.numAddresses());
		EXPECT_EQ(0u, sub.numKeys());
	}

	// endregion

	// region duration notification

	PLUGIN_TEST(CanPublishDurationNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<HashLockDurationNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();
		auto pTransaction = test::CreateTransaction<TTraits>();
		pTransaction->Duration = test::GenerateRandomValue<BlockDuration>();

		// Act:
		pPlugin->publish(*pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(pTransaction->Duration, notification.Duration);
	}

	// endregion

	// region mosaic notification

	PLUGIN_TEST(CanPublishMosaicNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<HashLockMosaicNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();
		auto pTransaction = test::CreateTransaction<TTraits>();
		pTransaction->Mosaic = { test::GenerateRandomValue<MosaicId>(), test::GenerateRandomValue<Amount>() };

		// Act:
		pPlugin->publish(*pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(pTransaction->Mosaic.MosaicId, notification.Mosaic.MosaicId);
		EXPECT_EQ(pTransaction->Mosaic.Amount, notification.Mosaic.Amount);
	}

	// endregion

	// region transaction hash notification

	namespace {
		template<typename TTransaction>
		void AssertHashLockNotification(const HashLockNotification& notification, const TTransaction& transaction) {
			test::AssertBaseLockNotification(notification, transaction);
			EXPECT_EQ(transaction.Hash, notification.Hash);
		}
	}

	PLUGIN_TEST(CanPublishTransactionHashNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<HashLockNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();
		auto pTransaction = test::CreateTransaction<TTraits>();

		// Act:
		pPlugin->publish(*pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		AssertHashLockNotification(notification, *pTransaction);
	}

	// endregion

	// region balance transfer

	PLUGIN_TEST(CanPublishBalanceReserveNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<model::BalanceReserveNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();
		auto pTransaction = test::CreateTransaction<TTraits>();

		// Act:
		pPlugin->publish(*pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(pTransaction->Signer, notification.Sender);
		EXPECT_EQ(pTransaction->Mosaic.MosaicId, notification.MosaicId);
		EXPECT_EQ(pTransaction->Mosaic.Amount, notification.Amount);
	}

	// endregion
}}
