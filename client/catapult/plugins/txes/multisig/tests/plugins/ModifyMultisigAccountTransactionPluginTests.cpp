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

#include "src/plugins/ModifyMultisigAccountTransactionPlugin.h"
#include "src/model/ModifyMultisigAccountTransaction.h"
#include "src/model/MultisigNotifications.h"
#include "catapult/model/Address.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"
#include <random>

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS ModifyMultisigAccountTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(ModifyMultisigAccount, 1, 1,)

		template<typename TTraits>
		auto CreateTransactionWithModifications(uint8_t numModifications) {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + numModifications * sizeof(CosignatoryModification);
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });

			pTransaction->Size = entitySize;
			pTransaction->ModificationsCount = numModifications;
			return pTransaction;
		}

		template<typename TTransaction>
		void SetAll(TTransaction& transaction, CosignatoryModificationType modificationType) {
			for (auto i = 0u; i < transaction.ModificationsCount; ++i)
				transaction.ModificationsPtr()[i].ModificationType = modificationType;
		}

		template<typename TTransaction>
		void AlternateAll(
				TTransaction& transaction,
				CosignatoryModificationType evenModificationType,
				CosignatoryModificationType oddModificationType) {
			for (auto i = 0u; i < transaction.ModificationsCount; ++i)
				transaction.ModificationsPtr()[i].ModificationType = 0 == i % 2 ? evenModificationType : oddModificationType;
		}
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ONLY_EMBEDDABLE(TEST_CLASS, , , Entity_Type_Modify_Multisig_Account)

	// endregion

	// region publish - no modifications

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNoModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(0);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			ModifyMultisigSettingsNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenNoModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(0);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		builder.template addExpectation<ModifyMultisigSettingsNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Signer);
			EXPECT_EQ(transaction.MinRemovalDelta, notification.MinRemovalDelta);
			EXPECT_EQ(transaction.MinApprovalDelta, notification.MinApprovalDelta);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region publish - add modifications

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenAddModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(2);
		SetAll(*pTransaction, CosignatoryModificationType::Add);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			ModifyMultisigNewCosignerNotification::Notification_Type,
			ModifyMultisigNewCosignerNotification::Notification_Type,
			ModifyMultisigCosignersNotification::Notification_Type,
			AddressInteractionNotification::Notification_Type,
			ModifyMultisigSettingsNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenAddModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(2);
		SetAll(*pTransaction, CosignatoryModificationType::Add);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		for (auto i = 0u; i < 2; ++i) {
			builder.template addExpectation<ModifyMultisigNewCosignerNotification>(i, [&transaction, i](const auto& notification) {
				EXPECT_EQ(transaction.Signer, notification.MultisigAccountKey);
				EXPECT_EQ(transaction.ModificationsPtr()[i].CosignatoryPublicKey, notification.CosignatoryKey);
			});
		}

		builder.template addExpectation<ModifyMultisigCosignersNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Signer);
			EXPECT_EQ(transaction.ModificationsCount, notification.ModificationsCount);
			EXPECT_EQ(transaction.ModificationsPtr(), notification.ModificationsPtr);
		});
		builder.template addExpectation<AddressInteractionNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Source);
			EXPECT_EQ(transaction.Type, notification.TransactionType);
			EXPECT_EQ(model::UnresolvedAddressSet(), notification.ParticipantsByAddress);

			utils::KeySet expectedParticipantsByKey{
				transaction.ModificationsPtr()[0].CosignatoryPublicKey,
				transaction.ModificationsPtr()[1].CosignatoryPublicKey
			};
			EXPECT_EQ(expectedParticipantsByKey, notification.ParticipantsByKey);
		});
		builder.template addExpectation<ModifyMultisigSettingsNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Signer);
			EXPECT_EQ(transaction.MinRemovalDelta, notification.MinRemovalDelta);
			EXPECT_EQ(transaction.MinApprovalDelta, notification.MinApprovalDelta);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region publish - del modifications

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenDelModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(2);
		SetAll(*pTransaction, CosignatoryModificationType::Del);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			ModifyMultisigCosignersNotification::Notification_Type,
			ModifyMultisigSettingsNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenDelModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(2);
		SetAll(*pTransaction, CosignatoryModificationType::Del);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		builder.template addExpectation<ModifyMultisigCosignersNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Signer);
			EXPECT_EQ(transaction.ModificationsCount, notification.ModificationsCount);
			EXPECT_EQ(transaction.ModificationsPtr(), notification.ModificationsPtr);
		});
		builder.template addExpectation<ModifyMultisigSettingsNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Signer);
			EXPECT_EQ(transaction.MinRemovalDelta, notification.MinRemovalDelta);
			EXPECT_EQ(transaction.MinApprovalDelta, notification.MinApprovalDelta);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region publish - add and del modifications

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenAddAndDelModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(4);
		AlternateAll(*pTransaction, CosignatoryModificationType::Add, CosignatoryModificationType::Del);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
			ModifyMultisigNewCosignerNotification::Notification_Type,
			ModifyMultisigNewCosignerNotification::Notification_Type,
			ModifyMultisigCosignersNotification::Notification_Type,
			AddressInteractionNotification::Notification_Type,
			ModifyMultisigSettingsNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotificationsWhenAddAndDelModifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(4);
		AlternateAll(*pTransaction, CosignatoryModificationType::Add, CosignatoryModificationType::Del);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		for (auto i = 0u; i < 2; ++i) {
			builder.template addExpectation<ModifyMultisigNewCosignerNotification>(i, [&transaction, i](const auto& notification) {
				EXPECT_EQ(transaction.Signer, notification.MultisigAccountKey);
				EXPECT_EQ(transaction.ModificationsPtr()[i * 2].CosignatoryPublicKey, notification.CosignatoryKey);
			});
		}

		builder.template addExpectation<ModifyMultisigCosignersNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Signer);
			EXPECT_EQ(transaction.ModificationsCount, notification.ModificationsCount);
			EXPECT_EQ(transaction.ModificationsPtr(), notification.ModificationsPtr);
		});
		builder.template addExpectation<AddressInteractionNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Source);
			EXPECT_EQ(transaction.Type, notification.TransactionType);
			EXPECT_EQ(model::UnresolvedAddressSet(), notification.ParticipantsByAddress);

			utils::KeySet expectedParticipantsByKey{
				transaction.ModificationsPtr()[0].CosignatoryPublicKey,
				transaction.ModificationsPtr()[2].CosignatoryPublicKey
			};
			EXPECT_EQ(expectedParticipantsByKey, notification.ParticipantsByKey);
		});
		builder.template addExpectation<ModifyMultisigSettingsNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Signer);
			EXPECT_EQ(transaction.MinRemovalDelta, notification.MinRemovalDelta);
			EXPECT_EQ(transaction.MinApprovalDelta, notification.MinApprovalDelta);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region additionalRequiredCosigners

	TEST(TEST_CLASS, CanExtractAdditionalRequiredCosignersFromEmbedded) {
		// Arrange:
		auto pPlugin = EmbeddedTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithModifications<EmbeddedTraits>(4);
		AlternateAll(*pTransaction, CosignatoryModificationType::Add, CosignatoryModificationType::Del);

		// Act:
		auto additionalCosigners = pPlugin->additionalRequiredCosigners(*pTransaction);

		// Assert:
		const auto* pModifications = pTransaction->ModificationsPtr();
		EXPECT_EQ(utils::KeySet({ pModifications[0].CosignatoryPublicKey, pModifications[2].CosignatoryPublicKey }), additionalCosigners);
	}

	// endregion
}}
