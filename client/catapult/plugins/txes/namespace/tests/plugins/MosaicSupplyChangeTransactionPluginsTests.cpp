#include "src/plugins/MosaicSupplyChangeTransactionPlugins.h"
#include "src/model/MosaicNotifications.h"
#include "src/model/MosaicSupplyChangeTransaction.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS MosaicSupplyChangeTransactionPluginsTests

	// region TransactionPlugin

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(MosaicSupplyChange);
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, EntityType::Mosaic_Supply_Change)

	PLUGIN_TEST(CanCalculateSize) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		transaction.Size = 0;

		// Act:
		auto realSize = pPlugin->calculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(typename TTraits::TransactionType), realSize);
	}

	PLUGIN_TEST(CanPublishCorrectNumberOfNotifications) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;

		// Act:
		pPlugin->publish(transaction, sub);

		// Assert:
		EXPECT_EQ(2u, sub.numNotifications());
	}

	PLUGIN_TEST(CanPublishMosaicChangeNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<MosaicChangeNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction.Signer);
		transaction.MosaicId = test::GenerateRandomValue<MosaicId>();

		// Act:
		pPlugin->publish(transaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(transaction.Signer, notification.Signer);
		EXPECT_EQ(transaction.MosaicId, notification.MosaicId);
	}

	PLUGIN_TEST(CanPublishMosaicSupplyChangeNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<MosaicSupplyChangeNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction.Signer);
		transaction.MosaicId = test::GenerateRandomValue<MosaicId>();
		transaction.Direction = MosaicSupplyChangeDirection::Increase;
		transaction.Delta = Amount(787);

		// Act:
		pPlugin->publish(transaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(transaction.Signer, notification.Signer);
		EXPECT_EQ(transaction.MosaicId, notification.MosaicId);
		EXPECT_EQ(MosaicSupplyChangeDirection::Increase, notification.Direction);
		EXPECT_EQ(Amount(787), notification.Delta);
	}

	// endregion
}}
