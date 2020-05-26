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

#include "src/plugins/AddressAliasTransactionPlugin.h"
#include "src/model/AddressAliasTransaction.h"
#include "src/model/AliasNotifications.h"
#include "src/model/NamespaceNotifications.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS AddressAliasTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(AddressAlias, 1, 1,)
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Alias_Address)

	// endregion

	// region publish

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrder) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			NamespaceRequiredNotification::Notification_Type,
			AliasLinkNotification::Notification_Type,
			AliasedAddressNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotifications) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

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
		builder.template addExpectation<AliasedAddressNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.NamespaceId, notification.NamespaceId);
			EXPECT_EQ(transaction.AliasAction, notification.AliasAction);
			EXPECT_EQ(transaction.Address, notification.AliasedData);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion
}}
