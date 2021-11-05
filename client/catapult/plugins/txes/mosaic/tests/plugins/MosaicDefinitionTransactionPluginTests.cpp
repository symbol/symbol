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

#include "src/plugins/MosaicDefinitionTransactionPlugin.h"
#include "src/model/MosaicDefinitionTransaction.h"
#include "src/model/MosaicNotifications.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/AddressTestUtils.h"
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
			HeightDependentAddress sinkAddress(test::GenerateRandomByteArray<Address>());
			sinkAddress.trySet(test::GenerateRandomByteArray<Address>(), Height(1111));
			sinkAddress.trySet(test::GenerateRandomByteArray<Address>(), Height(2222));

			return { UnresolvedMosaicId(1234), sinkAddress, fee, test::GenerateRandomByteArray<Key>() };
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
		auto Run_Test_Height = Height(1500);

		WeakEntityInfoT<Transaction> ToTransactionInfo(
				const MosaicDefinitionTransaction& transaction,
				const Hash256& hash,
				const BlockHeader& blockHeader) {
			return { transaction, hash, blockHeader };
		}

		WeakEntityInfoT<EmbeddedTransaction> ToTransactionInfo(
				const EmbeddedMosaicDefinitionTransaction& transaction,
				const Hash256& hash,
				const BlockHeader& blockHeader) {
			return { transaction, hash, blockHeader };
		}

		template<typename TBuilder, typename TTransaction>
		void RunTest(const TBuilder& builder, const TTransaction& transaction, const MosaicRentalFeeConfiguration& config) {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();

			BlockHeader blockHeader;
			blockHeader.Height = Run_Test_Height;

			// Act + Assert:
			builder.runTest(ToTransactionInfo(transaction, hash, blockHeader), config);
		}

		template<typename TTraits>
		void AddCommonExpectations(
				typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
				const MosaicRentalFeeConfiguration& config,
				const typename TTraits::TransactionType& transaction) {
			builder.template addExpectation<AccountAddressNotification>([&config](const auto& notification) {
				EXPECT_TRUE(notification.Address.isResolved());

				EXPECT_EQ(config.SinkAddress.get(Height(Run_Test_Height)), notification.Address.resolved());
			});
			builder.template addExpectation<MosaicNonceNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(GetSignerAddress(transaction), notification.Owner);
				EXPECT_EQ(transaction.Nonce, notification.MosaicNonce);
				EXPECT_EQ(transaction.Id, notification.MosaicId);
			});
			builder.template addExpectation<MosaicPropertiesNotification>([&transaction](const auto& notification) {
				auto expectedProperties = MosaicProperties(transaction.Flags, transaction.Divisibility, transaction.Duration);
				EXPECT_EQ(expectedProperties, notification.Properties);
			});
			builder.template addExpectation<MosaicDefinitionNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(GetSignerAddress(transaction), notification.Owner);
				EXPECT_EQ(transaction.Id, notification.MosaicId);

				auto expectedProperties = MosaicProperties(transaction.Flags, transaction.Divisibility, transaction.Duration);
				EXPECT_EQ(expectedProperties, notification.Properties);
			});
		}
	}

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNemesisIsSigner) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987));

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.SignerPublicKey = config.NemesisSignerPublicKey;

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			AccountAddressNotification::Notification_Type,
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
		transaction.SignerPublicKey = config.NemesisSignerPublicKey;

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, config, transaction);

		// Act + Assert:
		RunTest(builder, transaction, config);
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
			AccountAddressNotification::Notification_Type,
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
			EXPECT_TRUE(notification.Sender.isResolved());
			EXPECT_TRUE(notification.Recipient.isResolved());

			EXPECT_EQ(GetSignerAddress(transaction), notification.Sender.resolved());
			EXPECT_EQ(config.SinkAddress.get(Height(Run_Test_Height)), notification.Recipient.resolved());
			EXPECT_EQ(config.CurrencyMosaicId, notification.MosaicId);
			EXPECT_EQ(config.Fee, notification.Amount);
			EXPECT_EQ(BalanceTransferNotification::AmountType::Dynamic, notification.TransferAmountType);
		});
		builder.template addExpectation<MosaicRentalFeeNotification>([&config, &transaction](const auto& notification) {
			EXPECT_TRUE(notification.Sender.isResolved());
			EXPECT_TRUE(notification.Recipient.isResolved());

			EXPECT_EQ(GetSignerAddress(transaction), notification.Sender.resolved());
			EXPECT_EQ(config.SinkAddress.get(Height(Run_Test_Height)), notification.Recipient.resolved());
			EXPECT_EQ(config.CurrencyMosaicId, notification.MosaicId);
			EXPECT_EQ(config.Fee, notification.Amount);
		});

		// Act + Assert:
		RunTest(builder, transaction, config);
	}

	// endregion
}}
