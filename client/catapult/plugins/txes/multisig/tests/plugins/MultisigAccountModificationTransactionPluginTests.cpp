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

#include "src/plugins/MultisigAccountModificationTransactionPlugin.h"
#include "src/model/MultisigAccountModificationTransaction.h"
#include "src/model/MultisigNotifications.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"
#include <random>

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS MultisigAccountModificationTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(MultisigAccountModification, 1, 1,)

		template<typename TTraits>
		auto CreateTransactionWithModifications(uint8_t numAdditions, uint8_t numDeletions) {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + (numAdditions + numDeletions) * Address::Size;
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });

			pTransaction->Size = entitySize;
			pTransaction->AddressAdditionsCount = numAdditions;
			pTransaction->AddressDeletionsCount = numDeletions;
			return pTransaction;
		}
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ONLY_EMBEDDABLE(TEST_CLASS, , , Entity_Type_Multisig_Account_Modification)

	// endregion

	// region publish - no modifications

	namespace {
		template<typename TTraits>
		void AddCommonExpectations(
				typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
				const typename TTraits::TransactionType& transaction) {
			builder.template addExpectation<InternalPaddingNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.MultisigAccountModificationTransactionBody_Reserved1, notification.Padding);
			});
			builder.template addExpectation<MultisigSettingsNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(GetSignerAddress(transaction), notification.Multisig);
				EXPECT_EQ(transaction.MinRemovalDelta, notification.MinRemovalDelta);
				EXPECT_EQ(transaction.MinApprovalDelta, notification.MinApprovalDelta);
			});
		}
	}

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNoModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(0, 0);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			InternalPaddingNotification::Notification_Type,
			MultisigSettingsNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenNoModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(0, 0);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region publish - add modifications

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenAddModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(2, 0);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			InternalPaddingNotification::Notification_Type,
			AccountAddressNotification::Notification_Type,
			MultisigNewCosignatoryNotification::Notification_Type,
			AccountAddressNotification::Notification_Type,
			MultisigNewCosignatoryNotification::Notification_Type,
			MultisigCosignatoriesNotification::Notification_Type,
			AddressInteractionNotification::Notification_Type,
			MultisigSettingsNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenAddModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(2, 0);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);

		for (auto i = 0u; i < 2; ++i) {
			builder.template addExpectation<AccountAddressNotification>(i, [&transaction, i](const auto& notification) {
				EXPECT_FALSE(notification.Address.isResolved());

				EXPECT_EQ(transaction.AddressAdditionsPtr()[i], notification.Address.unresolved());
			});

			builder.template addExpectation<MultisigNewCosignatoryNotification>(i, [&transaction, i](const auto& notification) {
				EXPECT_EQ(GetSignerAddress(transaction), notification.Multisig);
				EXPECT_EQ(transaction.AddressAdditionsPtr()[i], notification.Cosignatory);
			});
		}

		builder.template addExpectation<MultisigCosignatoriesNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(GetSignerAddress(transaction), notification.Multisig);
			EXPECT_EQ(transaction.AddressAdditionsCount, notification.AddressAdditionsCount);
			EXPECT_EQ(transaction.AddressAdditionsPtr(), notification.AddressAdditionsPtr);
			EXPECT_EQ(transaction.AddressDeletionsCount, notification.AddressDeletionsCount);
			EXPECT_EQ(transaction.AddressDeletionsPtr(), notification.AddressDeletionsPtr);
		});
		builder.template addExpectation<AddressInteractionNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(GetSignerAddress(transaction), notification.Source);
			EXPECT_EQ(transaction.Type, notification.TransactionType);

			UnresolvedAddressSet expectedParticipantsByAddress{
				transaction.AddressAdditionsPtr()[0],
				transaction.AddressAdditionsPtr()[1]
			};
			EXPECT_EQ(expectedParticipantsByAddress, notification.ParticipantsByAddress);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region publish - del modifications

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenDelModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(0, 2);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			InternalPaddingNotification::Notification_Type,
			MultisigCosignatoriesNotification::Notification_Type,
			MultisigSettingsNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenDelModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(0, 2);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);

		builder.template addExpectation<MultisigCosignatoriesNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(GetSignerAddress(transaction), notification.Multisig);
			EXPECT_EQ(transaction.AddressAdditionsCount, notification.AddressAdditionsCount);
			EXPECT_EQ(transaction.AddressAdditionsPtr(), notification.AddressAdditionsPtr);
			EXPECT_EQ(transaction.AddressDeletionsCount, notification.AddressDeletionsCount);
			EXPECT_EQ(transaction.AddressDeletionsPtr(), notification.AddressDeletionsPtr);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region publish - add and del modifications

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenAddAndDelModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(2, 2);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			InternalPaddingNotification::Notification_Type,
			AccountAddressNotification::Notification_Type,
			MultisigNewCosignatoryNotification::Notification_Type,
			AccountAddressNotification::Notification_Type,
			MultisigNewCosignatoryNotification::Notification_Type,
			MultisigCosignatoriesNotification::Notification_Type,
			AddressInteractionNotification::Notification_Type,
			MultisigSettingsNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenAddAndDelModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(2, 2);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);

		for (auto i = 0u; i < 2; ++i) {
			builder.template addExpectation<AccountAddressNotification>(i, [&transaction, i](const auto& notification) {
				EXPECT_FALSE(notification.Address.isResolved());

				EXPECT_EQ(transaction.AddressAdditionsPtr()[i], notification.Address.unresolved());
			});

			builder.template addExpectation<MultisigNewCosignatoryNotification>(i, [&transaction, i](const auto& notification) {
				EXPECT_EQ(GetSignerAddress(transaction), notification.Multisig);
				EXPECT_EQ(transaction.AddressAdditionsPtr()[i], notification.Cosignatory);
			});
		}

		builder.template addExpectation<MultisigCosignatoriesNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(GetSignerAddress(transaction), notification.Multisig);
			EXPECT_EQ(transaction.AddressAdditionsCount, notification.AddressAdditionsCount);
			EXPECT_EQ(transaction.AddressAdditionsPtr(), notification.AddressAdditionsPtr);
			EXPECT_EQ(transaction.AddressDeletionsCount, notification.AddressDeletionsCount);
			EXPECT_EQ(transaction.AddressDeletionsPtr(), notification.AddressDeletionsPtr);
		});
		builder.template addExpectation<AddressInteractionNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(GetSignerAddress(transaction), notification.Source);
			EXPECT_EQ(transaction.Type, notification.TransactionType);

			UnresolvedAddressSet expectedParticipantsByAddress{
				transaction.AddressAdditionsPtr()[0],
				transaction.AddressAdditionsPtr()[1]
			};
			EXPECT_EQ(expectedParticipantsByAddress, notification.ParticipantsByAddress);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region additionalRequiredCosignatories

	TEST(TEST_CLASS, CanExtractAdditionalRequiredCosignatoriesFromEmbedded) {
		// Arrange:
		auto pPlugin = EmbeddedTraits::CreatePlugin();
		auto pTransaction = CreateTransactionWithModifications<EmbeddedTraits>(2, 2);

		// Act:
		auto additionalCosignatories = pPlugin->additionalRequiredCosignatories(*pTransaction);

		// Assert:
		const auto* pAddressAdditions = pTransaction->AddressAdditionsPtr();
		EXPECT_EQ(UnresolvedAddressSet({ pAddressAdditions[0], pAddressAdditions[1] }), additionalCosignatories);
	}

	// endregion
}}
