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

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(AccountLink, 1, 1,)
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Account_Link)

	// endregion

	// region publish - account link action link

	namespace {
		template<typename TTraits>
		void AddCommonExpectations(
				typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
				const typename TTraits::TransactionType& transaction) {
			builder.template addExpectation<AddressInteractionNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.SignerPublicKey, notification.Source);
				EXPECT_EQ(transaction.Type, notification.TransactionType);
				EXPECT_EQ(UnresolvedAddressSet(), notification.ParticipantsByAddress);
				EXPECT_EQ(utils::KeySet{ transaction.RemotePublicKey }, notification.ParticipantsByKey);
			});
			builder.template addExpectation<RemoteAccountLinkNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.SignerPublicKey, notification.MainAccountKey);
				EXPECT_EQ(transaction.RemotePublicKey, notification.RemoteAccountKey);
				EXPECT_EQ(transaction.LinkAction, notification.LinkAction);
			});
		}
	}

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenAccountLinkActionIsLink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.LinkAction = AccountLinkAction::Link;

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			NewRemoteAccountNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			AddressInteractionNotification::Notification_Type,
			RemoteAccountLinkNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenAccountLinkActionIsLink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.LinkAction = AccountLinkAction::Link;

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);
		builder.template addExpectation<AccountPublicKeyNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.RemotePublicKey, notification.PublicKey);
		});
		builder.template addExpectation<NewRemoteAccountNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.RemotePublicKey, notification.RemoteAccountKey);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region publish - account link action unlink

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenAccountLinkActionIsUnlink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.LinkAction = AccountLinkAction::Unlink;

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			AddressInteractionNotification::Notification_Type,
			RemoteAccountLinkNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenAccountLinkActionIsUnlink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.LinkAction = AccountLinkAction::Unlink;

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion
}}
