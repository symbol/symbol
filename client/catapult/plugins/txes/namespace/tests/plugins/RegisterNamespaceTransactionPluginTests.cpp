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

#include "src/plugins/RegisterNamespaceTransactionPlugin.h"
#include "src/model/NamespaceNotifications.h"
#include "src/model/RegisterNamespaceTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/constants.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS RegisterNamespaceTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_WITH_CONFIG_TEST_TRAITS(RegisterNamespace, NamespaceRentalFeeConfiguration, 1, 1,)

		NamespaceRentalFeeConfiguration CreateRentalFeeConfiguration(Amount rootFeePerBlock, Amount childFee) {
			return {
				test::GenerateRandomByteArray<Key>(),
				UnresolvedMosaicId(1234),
				test::GenerateRandomUnresolvedAddress(),
				rootFeePerBlock,
				childFee,
				test::GenerateRandomByteArray<Key>()
			};
		}

		template<typename TTraits>
		auto CreateTransactionWithName(uint8_t nameSize) {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + nameSize;
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			pTransaction->Size = entitySize;
			pTransaction->NamespaceNameSize = nameSize;
			test::FillWithRandomData(pTransaction->Signer);
			return pTransaction;
		}
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(
			TEST_CLASS,
			,
			,
			Entity_Type_Register_Namespace,
			CreateRentalFeeConfiguration(Amount(0), Amount(0)))

	// endregion

	// region publish - root nemesis signer

	namespace {
		template<typename TTraits>
		void AddCommonRootExpectations(
				typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
				const NamespaceRentalFeeConfiguration& config,
				const typename TTraits::TransactionType& transaction) {
			builder.template addExpectation<AccountPublicKeyNotification>([&config](const auto& notification) {
				EXPECT_EQ(config.SinkPublicKey, notification.PublicKey);
			});
			builder.template addExpectation<NamespaceNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.NamespaceType, notification.NamespaceType);
			});
			builder.template addExpectation<RootNamespaceNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.Signer, notification.Signer);
				EXPECT_EQ(transaction.NamespaceId, notification.NamespaceId);
				EXPECT_EQ(transaction.Duration, notification.Duration);
			});
			builder.template addExpectation<NamespaceNameNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.NamespaceId, notification.NamespaceId);
				EXPECT_EQ(Namespace_Base_Id, notification.ParentId);
				EXPECT_EQ(transaction.NamespaceNameSize, notification.NameSize);
				EXPECT_EQ(transaction.NamePtr(), notification.NamePtr);
			});
		}

		template<typename TTransaction>
		void PrepareRootNamespaceFiniteDurationTransaction(TTransaction& transaction) {
			test::FillWithRandomData(transaction);
			transaction.NamespaceType = NamespaceType::Root;
			transaction.Duration = BlockDuration(test::Random() | 1);
			transaction.NamespaceNameSize = 11;
		}
	}

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNemesisRootRegistration) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

		auto pTransaction = CreateTransactionWithName<TTraits>(11);
		PrepareRootNamespaceFiniteDurationTransaction(*pTransaction);
		pTransaction->Signer = config.NemesisPublicKey;

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			AccountPublicKeyNotification::Notification_Type,
			NamespaceNotification::Notification_Type,
			RootNamespaceNotification::Notification_Type,
			NamespaceNameNotification::Notification_Type
		}, config);
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenNemesisRootRegistration) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

		auto pTransaction = CreateTransactionWithName<TTraits>(11);
		PrepareRootNamespaceFiniteDurationTransaction(*pTransaction);
		pTransaction->Signer = config.NemesisPublicKey;

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonRootExpectations<TTraits>(builder, config, *pTransaction);

		// Act + Assert:
		builder.runTest(*pTransaction, config);
	}

	// endregion

	// region publish - root nemesis not signer (finite duration)

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNotNemesisRootRegistrationWithFiniteDuration) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

		auto pTransaction = CreateTransactionWithName<TTraits>(11);
		PrepareRootNamespaceFiniteDurationTransaction(*pTransaction);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			AccountPublicKeyNotification::Notification_Type,
			BalanceTransferNotification::Notification_Type,
			NamespaceRentalFeeNotification::Notification_Type,
			NamespaceNotification::Notification_Type,
			RootNamespaceNotification::Notification_Type,
			NamespaceNameNotification::Notification_Type
		}, config);
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenNotNemesisRootRegistrationWithFiniteDuration) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

		auto pTransaction = CreateTransactionWithName<TTraits>(11);
		PrepareRootNamespaceFiniteDurationTransaction(*pTransaction);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonRootExpectations<TTraits>(builder, config, transaction);
		builder.template addExpectation<BalanceTransferNotification>([&config, &transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Sender);
			EXPECT_EQ(config.SinkAddress, notification.Recipient);
			EXPECT_EQ(config.CurrencyMosaicId, notification.MosaicId);
			EXPECT_EQ(Amount(987 * transaction.Duration.unwrap()), notification.Amount);
		});
		builder.template addExpectation<NamespaceRentalFeeNotification>([&config, &transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Sender);
			EXPECT_EQ(config.SinkAddress, notification.Recipient);
			EXPECT_EQ(config.CurrencyMosaicId, notification.MosaicId);
			EXPECT_EQ(Amount(987 * transaction.Duration.unwrap()), notification.Amount);
		});

		// Act + Assert:
		builder.runTest(transaction, config);
	}

	// endregion

	// region publish - root nemesis not signer (eternal duration)

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNotNemesisRootRegistrationWithEternalDuration) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

		auto pTransaction = CreateTransactionWithName<TTraits>(11);
		PrepareRootNamespaceFiniteDurationTransaction(*pTransaction);
		pTransaction->Duration = BlockDuration();

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			AccountPublicKeyNotification::Notification_Type,
			NamespaceNotification::Notification_Type,
			RootNamespaceNotification::Notification_Type,
			NamespaceNameNotification::Notification_Type
		}, config);
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenNotNemesisRootRegistrationWithEternalDuration) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

		auto pTransaction = CreateTransactionWithName<TTraits>(11);
		PrepareRootNamespaceFiniteDurationTransaction(*pTransaction);
		pTransaction->Duration = BlockDuration();

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonRootExpectations<TTraits>(builder, config, *pTransaction);

		// Act + Assert:
		builder.runTest(*pTransaction, config);
	}

	// endregion

	// region publish - child nemesis signer

	namespace {
		template<typename TTraits>
		void AddCommonChildExpectations(
				typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
				const NamespaceRentalFeeConfiguration& config,
				const typename TTraits::TransactionType& transaction) {
			builder.template addExpectation<AccountPublicKeyNotification>([&config](const auto& notification) {
				EXPECT_EQ(config.SinkPublicKey, notification.PublicKey);
			});
			builder.template addExpectation<NamespaceNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.NamespaceType, notification.NamespaceType);
			});
			builder.template addExpectation<ChildNamespaceNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.Signer, notification.Signer);
				EXPECT_EQ(transaction.NamespaceId, notification.NamespaceId);
				EXPECT_EQ(transaction.ParentId, notification.ParentId);
			});
			builder.template addExpectation<NamespaceNameNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.NamespaceId, notification.NamespaceId);
				EXPECT_EQ(transaction.ParentId, notification.ParentId);
				EXPECT_EQ(transaction.NamespaceNameSize, notification.NameSize);
				EXPECT_EQ(transaction.NamePtr(), notification.NamePtr);
			});
		}

		template<typename TTransaction>
		void PrepareChildNamespaceTransaction(TTransaction& transaction) {
			test::FillWithRandomData(transaction);
			transaction.NamespaceType = NamespaceType::Child;
			transaction.ParentId = NamespaceId(test::Random() | 1);
			transaction.NamespaceNameSize = 11;
		}
	}

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNemesisChildRegistration) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

		auto pTransaction = CreateTransactionWithName<TTraits>(11);
		PrepareChildNamespaceTransaction(*pTransaction);
		pTransaction->Signer = config.NemesisPublicKey;

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			AccountPublicKeyNotification::Notification_Type,
			NamespaceNotification::Notification_Type,
			ChildNamespaceNotification::Notification_Type,
			NamespaceNameNotification::Notification_Type
		}, config);
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenNemesisChildRegistration) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

		auto pTransaction = CreateTransactionWithName<TTraits>(11);
		PrepareChildNamespaceTransaction(*pTransaction);
		pTransaction->Signer = config.NemesisPublicKey;

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonChildExpectations<TTraits>(builder, config, *pTransaction);

		// Act + Assert:
		builder.runTest(*pTransaction, config);
	}

	// endregion

	// region publish - child nemesis not signer

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNotNemesisChildRegistrationWithFiniteDuration) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

		auto pTransaction = CreateTransactionWithName<TTraits>(11);
		PrepareChildNamespaceTransaction(*pTransaction);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			AccountPublicKeyNotification::Notification_Type,
			BalanceTransferNotification::Notification_Type,
			NamespaceRentalFeeNotification::Notification_Type,
			NamespaceNotification::Notification_Type,
			ChildNamespaceNotification::Notification_Type,
			NamespaceNameNotification::Notification_Type
		}, config);
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenNotNemesisChildRegistrationWithFiniteDuration) {
		// Arrange:
		auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

		auto pTransaction = CreateTransactionWithName<TTraits>(11);
		PrepareChildNamespaceTransaction(*pTransaction);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonChildExpectations<TTraits>(builder, config, transaction);
		builder.template addExpectation<BalanceTransferNotification>([&config, &transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Sender);
			EXPECT_EQ(config.SinkAddress, notification.Recipient);
			EXPECT_EQ(config.CurrencyMosaicId, notification.MosaicId);
			EXPECT_EQ(Amount(777), notification.Amount);
		});
		builder.template addExpectation<NamespaceRentalFeeNotification>([&config, &transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Sender);
			EXPECT_EQ(config.SinkAddress, notification.Recipient);
			EXPECT_EQ(config.CurrencyMosaicId, notification.MosaicId);
			EXPECT_EQ(Amount(777), notification.Amount);
		});

		// Act + Assert:
		builder.runTest(transaction, config);
	}

	// endregion
}}
