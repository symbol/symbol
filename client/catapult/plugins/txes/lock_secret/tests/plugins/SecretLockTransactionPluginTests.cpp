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

#include "src/plugins/SecretLockTransactionPlugin.h"
#include "src/model/SecretLockNotifications.h"
#include "src/model/SecretLockTransaction.h"
#include "plugins/txes/lock_shared/tests/test/LockTransactionUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS SecretLockTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(SecretLock, 1, 1,)
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Secret_Lock)

	// endregion

	// region publish

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrder) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			AccountAddressNotification::Notification_Type,
			SecretLockDurationNotification::Notification_Type,
			SecretLockHashAlgorithmNotification::Notification_Type,
			AddressInteractionNotification::Notification_Type,
			BalanceDebitNotification::Notification_Type,
			SecretLockNotification::Notification_Type,
			BalanceTransferNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotifications) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		builder.template addExpectation<AccountAddressNotification>([&transaction](const auto& notification) {
			EXPECT_FALSE(notification.Address.isResolved());

			EXPECT_EQ(transaction.RecipientAddress, notification.Address.unresolved());
		});
		builder.template addExpectation<SecretLockDurationNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Duration, notification.Duration);
		});
		builder.template addExpectation<SecretLockHashAlgorithmNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.HashAlgorithm, notification.HashAlgorithm);
		});
		builder.template addExpectation<AddressInteractionNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(GetSignerAddress(transaction), notification.Source);
			EXPECT_EQ(transaction.Type, notification.TransactionType);
			EXPECT_EQ(UnresolvedAddressSet{ transaction.RecipientAddress }, notification.ParticipantsByAddress);
		});
		builder.template addExpectation<BalanceDebitNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(GetSignerAddress(transaction), notification.Sender);
			EXPECT_EQ(transaction.Mosaic.MosaicId, notification.MosaicId);
			EXPECT_EQ(transaction.Mosaic.Amount, notification.Amount);
		});
		builder.template addExpectation<SecretLockNotification>([&transaction](const auto& notification) {
			test::AssertBaseLockNotification(notification, transaction);
			EXPECT_EQ(transaction.HashAlgorithm, notification.HashAlgorithm);
			EXPECT_EQ(transaction.Secret, notification.Secret);
			EXPECT_EQ(transaction.RecipientAddress, notification.Recipient);
		});
		builder.template addExpectation<BalanceTransferNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(GetSignerAddress(transaction), notification.Sender);
			EXPECT_EQ(transaction.RecipientAddress, notification.Recipient);
			EXPECT_EQ(transaction.Mosaic.MosaicId, notification.MosaicId);
			EXPECT_EQ(Amount(0), notification.Amount);
			EXPECT_EQ(BalanceTransferNotification::AmountType::Static, notification.TransferAmountType);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion
}}
