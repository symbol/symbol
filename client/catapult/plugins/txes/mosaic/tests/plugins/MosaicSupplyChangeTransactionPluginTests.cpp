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

#include "src/plugins/MosaicSupplyChangeTransactionPlugin.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "src/model/MosaicNotifications.h"
#include "src/model/MosaicSupplyChangeTransaction.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS MosaicSupplyChangeTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(MosaicSupplyChange, 1, 1,)
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Mosaic_Supply_Change)

	// endregion

	// region publish

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrder) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			MosaicRequiredNotification::Notification_Type,
			MosaicSupplyChangeNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotifications) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		builder.template addExpectation<MosaicRequiredNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.SignerPublicKey, notification.Owner);
			EXPECT_EQ(MosaicId(), notification.MosaicId);
			EXPECT_EQ(transaction.MosaicId, notification.UnresolvedMosaicId);
			EXPECT_EQ(0u, notification.PropertyFlagMask);
			EXPECT_EQ(MosaicRequiredNotification::MosaicType::Unresolved, notification.ProvidedMosaicType);
		});
		builder.template addExpectation<MosaicSupplyChangeNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.SignerPublicKey, notification.Owner);
			EXPECT_EQ(transaction.MosaicId, notification.MosaicId);
			EXPECT_EQ(transaction.Action, notification.Action);
			EXPECT_EQ(transaction.Delta, notification.Delta);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion
}}
