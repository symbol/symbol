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

#include "src/plugins/MosaicAddressRestrictionTransactionPlugin.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "src/model/MosaicAddressRestrictionTransaction.h"
#include "src/model/MosaicRestrictionNotifications.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS MosaicAddressRestrictionTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(MosaicAddressRestriction, 1, 1,)
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Mosaic_Address_Restriction)

	// endregion

	// region publish

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrder) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			MosaicRequiredNotification::Notification_Type,
			MosaicRestrictionRequiredNotification::Notification_Type,
			MosaicAddressRestrictionModificationPreviousValueNotification::Notification_Type,
			MosaicAddressRestrictionModificationNewValueNotification::Notification_Type
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
			EXPECT_EQ(0x04u, notification.PropertyFlagMask);
			EXPECT_EQ(MosaicRequiredNotification::MosaicType::Unresolved, notification.ProvidedMosaicType);
		});
		builder.template addExpectation<MosaicRestrictionRequiredNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.MosaicId, notification.MosaicId);
			EXPECT_EQ(transaction.RestrictionKey, notification.RestrictionKey);
		});
		builder.template addExpectation<MosaicAddressRestrictionModificationPreviousValueNotification>([&transaction](
				const auto& notification) {
			EXPECT_EQ(transaction.MosaicId, notification.MosaicId);
			EXPECT_EQ(transaction.RestrictionKey, notification.RestrictionKey);
			EXPECT_EQ(transaction.TargetAddress, notification.TargetAddress);
			EXPECT_EQ(transaction.PreviousRestrictionValue, notification.RestrictionValue);
		});
		builder.template addExpectation<MosaicAddressRestrictionModificationNewValueNotification>([&transaction](
				const auto& notification) {
			EXPECT_EQ(transaction.MosaicId, notification.MosaicId);
			EXPECT_EQ(transaction.RestrictionKey, notification.RestrictionKey);
			EXPECT_EQ(transaction.TargetAddress, notification.TargetAddress);
			EXPECT_EQ(transaction.NewRestrictionValue, notification.RestrictionValue);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion
}}
