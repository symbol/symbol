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

#include "src/plugins/MosaicSupplyRevocationTransactionPlugin.h"
#include "src/model/MosaicFlags.h"
#include "src/model/MosaicSupplyRevocationTransaction.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS MosaicSupplyRevocationTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(MosaicSupplyRevocation, 1, 1,)
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Mosaic_Supply_Revocation)

	// endregion

	// region publish

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrder) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			MosaicRequiredNotification::Notification_Type,
			BalanceTransferNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotifications) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		builder.template addExpectation<MosaicRequiredNotification>([&transaction](const auto& notification) {
			EXPECT_TRUE(notification.Owner.isResolved());
			EXPECT_FALSE(notification.MosaicId.isResolved());

			EXPECT_EQ(GetSignerAddress(transaction), notification.Owner.resolved());
			EXPECT_EQ(transaction.Mosaic.MosaicId, notification.MosaicId.unresolved());
			EXPECT_EQ(utils::to_underlying_type(MosaicFlags::Revokable), notification.PropertyFlagMask);
		});
		builder.template addExpectation<BalanceTransferNotification>([&transaction](const auto& notification) {
			EXPECT_FALSE(notification.Sender.isResolved());
			EXPECT_TRUE(notification.Recipient.isResolved());

			EXPECT_EQ(transaction.SourceAddress, notification.Sender.unresolved());
			EXPECT_EQ(GetSignerAddress(transaction), notification.Recipient.resolved());
			EXPECT_EQ(transaction.Mosaic.MosaicId, notification.MosaicId);
			EXPECT_EQ(transaction.Mosaic.Amount, notification.Amount);
			EXPECT_EQ(BalanceTransferNotification::AmountType::Static, notification.TransferAmountType);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion
}}
