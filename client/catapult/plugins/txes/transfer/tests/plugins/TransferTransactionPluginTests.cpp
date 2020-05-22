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

#include "src/plugins/TransferTransactionPlugin.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "src/model/TransferNotifications.h"
#include "src/model/TransferTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS TransferTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(Transfer, 1, 1,)

		template<typename TTraits>
		auto CreateTransactionWithMosaics(uint8_t numMosaics, uint16_t messageSize = 0) {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + numMosaics * sizeof(Mosaic) + messageSize;
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });

			pTransaction->Size = entitySize;
			pTransaction->MessageSize = messageSize;
			pTransaction->MosaicsCount = numMosaics;
			return pTransaction;
		}
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Transfer)

	// endregion

	// region publish - neither message nor mosaics

	namespace {
		template<typename TTraits>
		void AddCommonExpectations(
				typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
				const typename TTraits::TransactionType& transaction) {
			builder.template addExpectation<InternalPaddingNotification>([&transaction](const auto& notification) {
				auto expectedPadding = transaction.TransferTransactionBody_Reserved1 << 8 | transaction.TransferTransactionBody_Reserved2;
				EXPECT_EQ(expectedPadding, notification.Padding);
			});
			builder.template addExpectation<AccountAddressNotification>([&transaction](const auto& notification) {
				EXPECT_FALSE(notification.Address.isResolved());

				EXPECT_EQ(transaction.RecipientAddress, notification.Address.unresolved());
			});
			builder.template addExpectation<AddressInteractionNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(GetSignerAddress(transaction), notification.Source);
				EXPECT_EQ(transaction.Type, notification.TransactionType);
				EXPECT_EQ(UnresolvedAddressSet{ transaction.RecipientAddress }, notification.ParticipantsByAddress);
			});
		}
	}

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNeitherMessageNorMosaicsArePresent) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.MessageSize = 0;
		transaction.MosaicsCount = 0;

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			InternalPaddingNotification::Notification_Type,
			AccountAddressNotification::Notification_Type,
			AddressInteractionNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenNeitherMessageNorMosaicsArePresent) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.MessageSize = 0;
		transaction.MosaicsCount = 0;

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region publish - message only

	namespace {
		template<typename TTransaction>
		void PrepareMessageOnlyTransaction(TTransaction& transaction) {
			test::FillWithRandomData(transaction);
			transaction.MosaicsCount = 0;
			transaction.MessageSize = 17;
			transaction.Size = static_cast<uint32_t>(TTransaction::CalculateRealSize(transaction));
		}
	}

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenMessageOnlyIsPresent) {
		// Arrange:
		auto pTransaction = CreateTransactionWithMosaics<TTraits>(0, 17);
		PrepareMessageOnlyTransaction(*pTransaction);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			InternalPaddingNotification::Notification_Type,
			AccountAddressNotification::Notification_Type,
			AddressInteractionNotification::Notification_Type,
			TransferMessageNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenMessageOnlyIsPresent) {
		// Arrange:
		auto pTransaction = CreateTransactionWithMosaics<TTraits>(0, 17);
		PrepareMessageOnlyTransaction(*pTransaction);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);
		builder.template addExpectation<TransferMessageNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.SignerPublicKey, notification.SenderPublicKey);
			EXPECT_EQ(transaction.RecipientAddress, notification.Recipient);
			EXPECT_EQ(17u, notification.MessageSize);
			EXPECT_EQ(transaction.MessagePtr(), notification.MessagePtr);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region publish - mosaics only

	namespace {
		template<typename TTransaction>
		void PrepareMosaicsOnlyTransaction(TTransaction& transaction) {
			test::FillWithRandomData(transaction);
			transaction.MosaicsCount = 2;
			transaction.MessageSize = 0;
			transaction.Size = static_cast<uint32_t>(TTransaction::CalculateRealSize(transaction));
		}
	}

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenMosaicsOnlyArePresent) {
		// Arrange:
		auto pTransaction = CreateTransactionWithMosaics<TTraits>(2, 0);
		PrepareMosaicsOnlyTransaction(*pTransaction);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			InternalPaddingNotification::Notification_Type,
			AccountAddressNotification::Notification_Type,
			AddressInteractionNotification::Notification_Type,
			BalanceTransferNotification::Notification_Type,
			BalanceTransferNotification::Notification_Type,
			TransferMosaicsNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenMosaicsOnlyArePresent) {
		// Arrange:
		auto pTransaction = CreateTransactionWithMosaics<TTraits>(2, 0);
		PrepareMosaicsOnlyTransaction(*pTransaction);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);
		for (auto i = 0u; i < 2u; ++i) {
			builder.template addExpectation<BalanceTransferNotification>(i, [&transaction, i](const auto& notification) {
				EXPECT_EQ(GetSignerAddress(transaction), notification.Sender);
				EXPECT_EQ(transaction.MosaicsPtr()[i].MosaicId, notification.MosaicId);
				EXPECT_EQ(transaction.MosaicsPtr()[i].Amount, notification.Amount);
				EXPECT_EQ(transaction.RecipientAddress, notification.Recipient);
				EXPECT_EQ(BalanceTransferNotification::AmountType::Static, notification.TransferAmountType);
			});
		}

		builder.template addExpectation<TransferMosaicsNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.MosaicsCount, notification.MosaicsCount);
			EXPECT_EQ(transaction.MosaicsPtr(), notification.MosaicsPtr);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region publish - both message and mosaics

	namespace {
		template<typename TTransaction>
		void PrepareMessageAndMosaicsTransaction(TTransaction& transaction) {
			test::FillWithRandomData(transaction);
			transaction.MosaicsCount = 3;
			transaction.MessageSize = 17;
			transaction.Size = static_cast<uint32_t>(TTransaction::CalculateRealSize(transaction));
		}
	}

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenMessageAndMosaicsArePresent) {
		// Arrange:
		auto pTransaction = CreateTransactionWithMosaics<TTraits>(3, 17);
		PrepareMessageAndMosaicsTransaction(*pTransaction);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			InternalPaddingNotification::Notification_Type,
			AccountAddressNotification::Notification_Type,
			AddressInteractionNotification::Notification_Type,
			BalanceTransferNotification::Notification_Type,
			BalanceTransferNotification::Notification_Type,
			BalanceTransferNotification::Notification_Type,
			TransferMessageNotification::Notification_Type,
			TransferMosaicsNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenMessageAndMosaicsArePresent) {
		// Arrange:
		auto pTransaction = CreateTransactionWithMosaics<TTraits>(3, 17);
		PrepareMessageAndMosaicsTransaction(*pTransaction);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);
		for (auto i = 0u; i < 3u; ++i) {
			builder.template addExpectation<BalanceTransferNotification>(i, [&transaction, i](const auto& notification) {
				EXPECT_EQ(GetSignerAddress(transaction), notification.Sender);
				EXPECT_EQ(transaction.MosaicsPtr()[i].MosaicId, notification.MosaicId);
				EXPECT_EQ(transaction.MosaicsPtr()[i].Amount, notification.Amount);
				EXPECT_EQ(transaction.RecipientAddress, notification.Recipient);
				EXPECT_EQ(BalanceTransferNotification::AmountType::Static, notification.TransferAmountType);
			});
		}

		builder.template addExpectation<TransferMessageNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.SignerPublicKey, notification.SenderPublicKey);
			EXPECT_EQ(transaction.RecipientAddress, notification.Recipient);
			EXPECT_EQ(17u, notification.MessageSize);
			EXPECT_EQ(transaction.MessagePtr(), notification.MessagePtr);
		});
		builder.template addExpectation<TransferMosaicsNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.MosaicsCount, notification.MosaicsCount);
			EXPECT_EQ(transaction.MosaicsPtr(), notification.MosaicsPtr);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion
}}
