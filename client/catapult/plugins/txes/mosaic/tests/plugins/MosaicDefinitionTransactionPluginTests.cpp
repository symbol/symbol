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

#include "src/plugins/MosaicDefinitionTransactionPlugin.h"
#include "src/model/MosaicDefinitionTransaction.h"
#include "src/model/MosaicNotifications.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS MosaicDefinitionTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_WITH_CONFIG_TEST_TRAITS(MosaicDefinition, MosaicRentalFeeConfiguration, 1, 1,)

		MosaicRentalFeeConfiguration CreateRentalFeeConfiguration(Amount fee) {
			return {
				test::GenerateRandomByteArray<Key>(),
				UnresolvedMosaicId(1234),
				test::GenerateRandomUnresolvedAddress(),
				fee,
				test::GenerateRandomByteArray<Key>()
			};
		}
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(
			TEST_CLASS,
			,
			,
			Entity_Type_Mosaic_Definition,
			CreateRentalFeeConfiguration(Amount(0)))

	// endregion

	// region publish - nemesis signer

	namespace {
		template<typename TTraits>
		void AddCommonExpectations(
				typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
				const MosaicRentalFeeConfiguration& config,
				const typename TTraits::TransactionType& transaction) {
			builder.template addExpectation<AccountPublicKeyNotification>([&config](const auto& notification) {
				EXPECT_EQ(config.SinkPublicKey, notification.PublicKey);
			});
			builder.template addExpectation<MosaicNonceNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.SignerPublicKey, notification.Owner);
				EXPECT_EQ(transaction.Nonce, notification.MosaicNonce);
				EXPECT_EQ(transaction.Id, notification.MosaicId);
			});
			builder.template addExpectation<MosaicPropertiesNotification>([&transaction](const auto& notification) {
				auto expectedProperties = model::MosaicProperties(transaction.Flags, transaction.Divisibility, transaction.Duration);
				EXPECT_EQ(expectedProperties, notification.Properties);
			});
			builder.template addExpectation<MosaicDefinitionNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.SignerPublicKey, notification.Owner);
				EXPECT_EQ(transaction.Id, notification.MosaicId);

				auto expectedProperties = model::MosaicProperties(transaction.Flags, transaction.Divisibility, transaction.Duration);
				EXPECT_EQ(expectedProperties, notification.Properties);
			});
		}
	}

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNemesisIsSigner) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987));

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.SignerPublicKey = config.NemesisPublicKey;

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			AccountPublicKeyNotification::Notification_Type,
			MosaicNonceNotification::Notification_Type,
			MosaicPropertiesNotification::Notification_Type,
			MosaicDefinitionNotification::Notification_Type
		}, config);
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenNemesisIsSigner) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987));

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.SignerPublicKey = config.NemesisPublicKey;

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, config, transaction);

		// Act + Assert:
		builder.runTest(transaction, config);
	}

	// endregion

	// region publish - nemesis not signer

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNemesisIsNotSigner) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987));

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			AccountPublicKeyNotification::Notification_Type,
			BalanceTransferNotification::Notification_Type,
			MosaicRentalFeeNotification::Notification_Type,
			MosaicNonceNotification::Notification_Type,
			MosaicPropertiesNotification::Notification_Type,
			MosaicDefinitionNotification::Notification_Type
		}, config);
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenNemesisIsNotSigner) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987));

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, config, transaction);
		builder.template addExpectation<BalanceTransferNotification>([&config, &transaction](const auto& notification) {
			EXPECT_EQ(transaction.SignerPublicKey, notification.Sender);
			EXPECT_EQ(config.SinkAddress, notification.Recipient);
			EXPECT_EQ(config.CurrencyMosaicId, notification.MosaicId);
			EXPECT_EQ(config.Fee, notification.Amount);
			EXPECT_EQ(BalanceTransferNotification::AmountType::Dynamic, notification.TransferAmountType);
		});
		builder.template addExpectation<MosaicRentalFeeNotification>([&config, &transaction](const auto& notification) {
			EXPECT_EQ(transaction.SignerPublicKey, notification.Sender);
			EXPECT_EQ(config.SinkAddress, notification.Recipient);
			EXPECT_EQ(config.CurrencyMosaicId, notification.MosaicId);
			EXPECT_EQ(config.Fee, notification.Amount);
		});

		// Act + Assert:
		builder.runTest(transaction, config);
	}

	// endregion
}}
