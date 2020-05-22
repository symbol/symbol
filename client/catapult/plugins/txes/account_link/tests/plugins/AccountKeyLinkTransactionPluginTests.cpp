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

#include "src/plugins/AccountKeyLinkTransactionPlugin.h"
#include "src/model/AccountKeyLinkTransaction.h"
#include "src/model/AccountLinkNotifications.h"
#include "catapult/model/Address.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS AccountKeyLinkTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(AccountKeyLink, 1, 1,)
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Account_Key_Link)

	// endregion

	// region publish - link action link

	namespace {
		template<typename TTraits>
		void AddCommonExpectations(
				typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
				const typename TTraits::TransactionType& transaction) {
			builder.template addExpectation<KeyLinkActionNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.LinkAction, notification.LinkAction);
			});
			builder.template addExpectation<AddressInteractionNotification>([&transaction](const auto& notification) {
				auto linkedAddress = PublicKeyToAddress(transaction.LinkedPublicKey, transaction.Network);
				EXPECT_EQ(GetSignerAddress(transaction), notification.Source);
				EXPECT_EQ(transaction.Type, notification.TransactionType);
				EXPECT_EQ(UnresolvedAddressSet{ linkedAddress.template copyTo<UnresolvedAddress>() }, notification.ParticipantsByAddress);
			});
			builder.template addExpectation<RemoteAccountKeyLinkNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.SignerPublicKey, notification.MainAccountPublicKey);
				EXPECT_EQ(transaction.LinkedPublicKey, notification.LinkedPublicKey);
				EXPECT_EQ(transaction.LinkAction, notification.LinkAction);
			});
		}
	}

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenLinkActionIsLink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.LinkAction = LinkAction::Link;

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			NewRemoteAccountNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			KeyLinkActionNotification::Notification_Type,
			AddressInteractionNotification::Notification_Type,
			RemoteAccountKeyLinkNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenLinkActionIsLink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.LinkAction = LinkAction::Link;

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);
		builder.template addExpectation<AccountPublicKeyNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.LinkedPublicKey, notification.PublicKey);
		});
		builder.template addExpectation<NewRemoteAccountNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.LinkedPublicKey, notification.LinkedPublicKey);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region publish - link action unlink

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenLinkActionIsUnlink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.LinkAction = LinkAction::Unlink;

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			KeyLinkActionNotification::Notification_Type,
			AddressInteractionNotification::Notification_Type,
			RemoteAccountKeyLinkNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenLinkActionIsUnlink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.LinkAction = LinkAction::Unlink;

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion
}}
