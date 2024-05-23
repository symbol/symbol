/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "src/model/AliasNotifications.h"
#include "src/model/MosaicAliasTransaction.h"
#include "src/model/NamespaceNotifications.h"
#include "src/plugins/MosaicAliasTransactionPlugin.h"
#include "tests/TestHarness.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"

using namespace catapult::model;

namespace catapult {
namespace plugins {

#define TEST_CLASS MosaicAliasTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(MosaicAlias, 1, 1, )
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Alias_Mosaic)

	// endregion

	// region publish

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrder_Link) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.AliasAction = AliasAction::Link;

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(
			transaction,
			{ NamespaceRequiredNotification::Notification_Type,
				AliasLinkNotification::Notification_Type,
				AliasedMosaicIdNotification::Notification_Type,
				MosaicRequiredNotification::Notification_Type });
	}

	PLUGIN_TEST(CanPublishAllNotifications_Link) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.AliasAction = AliasAction::Link;

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		builder.template addExpectation<NamespaceRequiredNotification>([&transaction](const auto& notification) {
			EXPECT_TRUE(notification.Owner.isResolved());

			EXPECT_EQ(GetSignerAddress(transaction), notification.Owner.resolved());
			EXPECT_EQ(transaction.NamespaceId, notification.NamespaceId);
		});
		builder.template addExpectation<AliasLinkNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.NamespaceId, notification.NamespaceId);
			EXPECT_EQ(transaction.AliasAction, notification.AliasAction);
		});
		builder.template addExpectation<AliasedMosaicIdNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.NamespaceId, notification.NamespaceId);
			EXPECT_EQ(transaction.AliasAction, notification.AliasAction);
			EXPECT_EQ(transaction.MosaicId, notification.AliasedData);
		});
		builder.template addExpectation<MosaicRequiredNotification>([&transaction](const auto& notification) {
			EXPECT_TRUE(notification.Owner.isResolved());
			EXPECT_TRUE(notification.MosaicId.isResolved());

			EXPECT_EQ(GetSignerAddress(transaction), notification.Owner.resolved());
			EXPECT_EQ(transaction.MosaicId, notification.MosaicId.resolved());
			EXPECT_EQ(0u, notification.PropertyFlagMask);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrder_Unlink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.AliasAction = AliasAction::Unlink;

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(
			transaction,
			{ NamespaceRequiredNotification::Notification_Type,
				AliasLinkNotification::Notification_Type,
				AliasedMosaicIdNotification::Notification_Type });
	}

	PLUGIN_TEST(CanPublishAllNotifications_Unlink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.AliasAction = AliasAction::Unlink;

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		builder.template addExpectation<NamespaceRequiredNotification>([&transaction](const auto& notification) {
			EXPECT_TRUE(notification.Owner.isResolved());

			EXPECT_EQ(GetSignerAddress(transaction), notification.Owner.resolved());
			EXPECT_EQ(transaction.NamespaceId, notification.NamespaceId);
		});
		builder.template addExpectation<AliasLinkNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.NamespaceId, notification.NamespaceId);
			EXPECT_EQ(transaction.AliasAction, notification.AliasAction);
		});
		builder.template addExpectation<AliasedMosaicIdNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.NamespaceId, notification.NamespaceId);
			EXPECT_EQ(transaction.AliasAction, notification.AliasAction);
			EXPECT_EQ(transaction.MosaicId, notification.AliasedData);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion
}
}
