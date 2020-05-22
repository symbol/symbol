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

#include "src/plugins/MosaicGlobalRestrictionTransactionPlugin.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "src/model/MosaicGlobalRestrictionTransaction.h"
#include "src/model/MosaicRestrictionNotifications.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS MosaicGlobalRestrictionTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(MosaicGlobalRestriction, 1, 1,)
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Mosaic_Global_Restriction)

	// endregion

	// region publish - with reference mosaic id

	namespace {
		template<typename TTraits>
		void AddCommonExpectations(
				typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
				const typename TTraits::TransactionType& transaction) {
			builder.template addExpectation<MosaicRestrictionTypeNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.NewRestrictionType, notification.RestrictionType);
			});
			builder.template addExpectation<MosaicRequiredNotification>([&transaction](const auto& notification) {
				EXPECT_TRUE(notification.Owner.isResolved());
				EXPECT_FALSE(notification.MosaicId.isResolved());

				EXPECT_EQ(GetSignerAddress(transaction), notification.Owner.resolved());
				EXPECT_EQ(transaction.MosaicId, notification.MosaicId.unresolved());
				EXPECT_EQ(0x04u, notification.PropertyFlagMask);
			});
			builder.template addExpectation<MosaicGlobalRestrictionModificationPreviousValueNotification>([&transaction](
					const auto& notification) {
				EXPECT_EQ(transaction.MosaicId, notification.MosaicId);
				EXPECT_EQ(transaction.ReferenceMosaicId, notification.ReferenceMosaicId);
				EXPECT_EQ(transaction.RestrictionKey, notification.RestrictionKey);
				EXPECT_EQ(transaction.PreviousRestrictionValue, notification.RestrictionValue);
				EXPECT_EQ(transaction.PreviousRestrictionType, notification.RestrictionType);
			});
			builder.template addExpectation<MosaicGlobalRestrictionModificationNewValueNotification>([&transaction](
					const auto& notification) {
				EXPECT_EQ(transaction.MosaicId, notification.MosaicId);
				EXPECT_EQ(transaction.ReferenceMosaicId, notification.ReferenceMosaicId);
				EXPECT_EQ(transaction.RestrictionKey, notification.RestrictionKey);
				EXPECT_EQ(transaction.NewRestrictionValue, notification.RestrictionValue);
				EXPECT_EQ(transaction.NewRestrictionType, notification.RestrictionType);
			});
		}
	}

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenReferenceMosaicIdIsProvided) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.ReferenceMosaicId = UnresolvedMosaicId(test::Random() | 1);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			MosaicRestrictionTypeNotification::Notification_Type,
			MosaicRequiredNotification::Notification_Type,
			MosaicRestrictionRequiredNotification::Notification_Type,
			MosaicGlobalRestrictionModificationPreviousValueNotification::Notification_Type,
			MosaicGlobalRestrictionModificationNewValueNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenReferenceMosaicIdIsProvided) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.ReferenceMosaicId = UnresolvedMosaicId(test::Random() | 1);

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);
		builder.template addExpectation<MosaicRestrictionRequiredNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.ReferenceMosaicId, notification.MosaicId);
			EXPECT_EQ(transaction.RestrictionKey, notification.RestrictionKey);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region publish - without reference mosaic id

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenReferenceMosaicIdIsNotProvided) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.ReferenceMosaicId = UnresolvedMosaicId();

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			MosaicRestrictionTypeNotification::Notification_Type,
			MosaicRequiredNotification::Notification_Type,
			MosaicGlobalRestrictionModificationPreviousValueNotification::Notification_Type,
			MosaicGlobalRestrictionModificationNewValueNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenReferenceMosaicIdIsNotProvided) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.ReferenceMosaicId = UnresolvedMosaicId();

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion
}}
