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

#include "src/plugins/AccountLinkTransactionPlugin.h"
#include "src/model/AccountLinkNotifications.h"
#include "src/model/AccountLinkTransaction.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS AccountLinkTransactionPluginTests

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(AccountLink, 2, 2,)
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Account_Link)

	// region basic tests

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

	PLUGIN_TEST(CanExtractAccounts) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction.Signer);
		test::FillWithRandomData(transaction.RemoteAccountKey);
		transaction.LinkAction = AccountLinkAction::Link;

		// Act:
		test::PublishTransaction(*pPlugin, transaction, sub);

		// Assert:
		EXPECT_EQ(4u, sub.numNotifications());
		EXPECT_EQ(0u, sub.numAddresses());
		EXPECT_EQ(1u, sub.numKeys());

		EXPECT_TRUE(sub.contains(transaction.RemoteAccountKey));
	}

	// endregion

	// region single notification tests

	PLUGIN_TEST(CanExtractNewRemoteAccount) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<NewRemoteAccountNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction.Signer);
		test::FillWithRandomData(transaction.RemoteAccountKey);
		transaction.LinkAction = AccountLinkAction::Link;

		// Act:
		test::PublishTransaction(*pPlugin, transaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		EXPECT_EQ(transaction.RemoteAccountKey, sub.matchingNotifications()[0].RemoteAccountKey);
	}

	PLUGIN_TEST(CanExtractAddressInteraction) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<AddressInteractionNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction.Signer);
		transaction.Type = static_cast<EntityType>(0x0815);
		test::FillWithRandomData(transaction.RemoteAccountKey);

		// Act:
		test::PublishTransaction(*pPlugin, transaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(transaction.Signer, notification.Source);
		EXPECT_EQ(transaction.Type, notification.TransactionType);
		EXPECT_EQ(UnresolvedAddressSet(), notification.ParticipantsByAddress);
		EXPECT_EQ(utils::KeySet{ transaction.RemoteAccountKey }, notification.ParticipantsByKey);
	}

	PLUGIN_TEST(CanExtractRemoteAccountLink) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<RemoteAccountLinkNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction.Signer);
		test::FillWithRandomData(transaction.RemoteAccountKey);
		transaction.LinkAction = AccountLinkAction::Unlink;

		// Act:
		test::PublishTransaction(*pPlugin, transaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(transaction.Signer, notification.MainAccountKey);
		EXPECT_EQ(transaction.RemoteAccountKey, notification.RemoteAccountKey);
		EXPECT_EQ(transaction.LinkAction, notification.LinkAction);
	}

	// endregion

	// region multiple notification tests

	PLUGIN_TEST(CanExtractAllNotificationsWhenAccountLinkActionIsLink) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		transaction.LinkAction = AccountLinkAction::Link;

		// Act:
		test::PublishTransaction(*pPlugin, transaction, sub);

		// Assert:
		const auto& notificationTypes = sub.notificationTypes();
		ASSERT_EQ(4u, notificationTypes.size());
		EXPECT_EQ(AccountLink_New_Remote_Account_Notification, notificationTypes[0]);
		EXPECT_EQ(Core_Register_Account_Public_Key_Notification, notificationTypes[1]);
		EXPECT_EQ(Core_Address_Interaction_Notification, notificationTypes[2]);
		EXPECT_EQ(AccountLink_Remote_Notification, notificationTypes[3]);
	}

	PLUGIN_TEST(CanExtractAllNotificationsWhenAccountLinkActionIsUnlink) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		transaction.LinkAction = AccountLinkAction::Unlink;

		// Act:
		test::PublishTransaction(*pPlugin, transaction, sub);

		// Assert:
		const auto& notificationTypes = sub.notificationTypes();
		ASSERT_EQ(3u, notificationTypes.size());
		EXPECT_EQ(Core_Register_Account_Public_Key_Notification, notificationTypes[0]);
		EXPECT_EQ(Core_Address_Interaction_Notification, notificationTypes[1]);
		EXPECT_EQ(AccountLink_Remote_Notification, notificationTypes[2]);
	}

	// endregion
}}
