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

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(ModifyMultisigAccount, 1, 1,)

		template<typename TTraits>
		auto CreateTransactionWithModifications(uint8_t numModifications) {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + numModifications * sizeof(CosignatoryModification);
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			pTransaction->Size = entitySize;
			pTransaction->ModificationsCount = numModifications;
			test::FillWithRandomData(pTransaction->Signer);
			return pTransaction;
		}
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ONLY_EMBEDDABLE(TEST_CLASS, , , Entity_Type_Modify_Multisig_Account)

	PLUGIN_TEST(CanCalculateSize) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		transaction.Size = 0;
		transaction.ModificationsCount = 7;

		// Act:
		auto realSize = pPlugin->calculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 7 * sizeof(CosignatoryModification), realSize);
	}

	// region publish - basic

	namespace {
		template<typename TTraits, typename TAdditionalAssert>
		void AssertNumNotifications(
				size_t numExpectedNotifications,
				const typename TTraits::TransactionType& transaction,
				TAdditionalAssert additionalAssert) {
			// Arrange:
			mocks::MockNotificationSubscriber sub;
			auto pPlugin = TTraits::CreatePlugin();

			// Act:
			test::PublishTransaction(*pPlugin, transaction, sub);

			// Assert:
			ASSERT_EQ(numExpectedNotifications, sub.numNotifications());
			additionalAssert(sub);
		}

		template<typename TTraits>
		void AssertNumNotifications(size_t numExpectedNotifications, const typename TTraits::TransactionType& transaction) {
			// Assert:
			AssertNumNotifications<TTraits>(numExpectedNotifications, transaction, [](const auto&) {});
		}
	}

	PLUGIN_TEST(CanPublishCorrectNumberOfNotificationsWhenNoModificationsArePresent) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(0);

		// Assert:
		AssertNumNotifications<TTraits>(1, *pTransaction);
	}

	PLUGIN_TEST(CanPublishCorrectNumberOfNotificationsWhenNoAddModificationsArePresent) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(3);
		auto* pModification = pTransaction->ModificationsPtr();
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomByteArray<Key>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomByteArray<Key>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomByteArray<Key>() };

		// Assert:
		AssertNumNotifications<TTraits>(2, *pTransaction);
	}

	PLUGIN_TEST(CanPublishCorrectNumberOfNotificationsWhenAddModificationsArePresent) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(3);
		auto* pModification = pTransaction->ModificationsPtr();
		*pModification++ = { CosignatoryModificationType::Add, test::GenerateRandomByteArray<Key>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomByteArray<Key>() };
		*pModification++ = { CosignatoryModificationType::Add, test::GenerateRandomByteArray<Key>() };

		// Assert:
		AssertNumNotifications<TTraits>(5, *pTransaction, [](const auto& sub) {
			// - multisig modify new cosigner notifications must be the first raised notifications
			EXPECT_EQ(Multisig_Modify_New_Cosigner_Notification, sub.notificationTypes()[0]);
			EXPECT_EQ(Multisig_Modify_New_Cosigner_Notification, sub.notificationTypes()[1]);
		});
	}

	// endregion

	// region publish - modify multisig settings notification

	PLUGIN_TEST(CanPublishSettingsNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<ModifyMultisigSettingsNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		transaction.ModificationsCount = 0;
		test::FillWithRandomData(transaction.Signer);
		transaction.MinRemovalDelta = -3;
		transaction.MinApprovalDelta = 4;

		// Act:
		test::PublishTransaction(*pPlugin, transaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(transaction.Signer, notification.Signer);
		EXPECT_EQ(-3, notification.MinRemovalDelta);
		EXPECT_EQ(4, notification.MinApprovalDelta);
	}

	// endregion

	// region publish - modify multisig cosigners notification

	PLUGIN_TEST(CanPublishCosignersNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<ModifyMultisigCosignersNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithModifications<TTraits>(3);
		auto* pModification = pTransaction->ModificationsPtr();
		*pModification++ = { CosignatoryModificationType::Add, test::GenerateRandomByteArray<Key>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomByteArray<Key>() };
		*pModification++ = { CosignatoryModificationType::Add, test::GenerateRandomByteArray<Key>() };

		// Act:
		test::PublishTransaction(*pPlugin, *pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(pTransaction->Signer, notification.Signer);
		EXPECT_EQ(3, notification.ModificationsCount);
		EXPECT_EQ(pTransaction->ModificationsPtr(), notification.ModificationsPtr);
	}

	PLUGIN_TEST(NoCosignersNotificationWhenNoModificationIsPresent) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<ModifyMultisigCosignersNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		transaction.ModificationsCount = 0;

		// Act:
		test::PublishTransaction(*pPlugin, transaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numNotifications());
		ASSERT_EQ(0u, sub.numMatchingNotifications());
	}

	// endregion

	// region publish - modify multisig new cosigner notification

	PLUGIN_TEST(CanPublishNewCosignerNotificationForEachAddModification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<ModifyMultisigNewCosignerNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithModifications<TTraits>(3);
		auto* pModification = pTransaction->ModificationsPtr();
		*pModification++ = { CosignatoryModificationType::Add, test::GenerateRandomByteArray<Key>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomByteArray<Key>() };
		*pModification++ = { CosignatoryModificationType::Add, test::GenerateRandomByteArray<Key>() };

		// Act:
		test::PublishTransaction(*pPlugin, *pTransaction, sub);

		// Assert:
		ASSERT_EQ(2u, sub.numMatchingNotifications());

		EXPECT_EQ(pTransaction->Signer, sub.matchingNotifications()[0].MultisigAccountKey);
		EXPECT_EQ(pTransaction->ModificationsPtr()[0].CosignatoryPublicKey, sub.matchingNotifications()[0].CosignatoryKey);

		EXPECT_EQ(pTransaction->Signer, sub.matchingNotifications()[1].MultisigAccountKey);
		EXPECT_EQ(pTransaction->ModificationsPtr()[2].CosignatoryPublicKey, sub.matchingNotifications()[1].CosignatoryKey);
	}

	PLUGIN_TEST(NoNewCosignerNotificationsWhenNoAddModificationsArePresent) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<ModifyMultisigNewCosignerNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithModifications<TTraits>(3);
		auto* pModification = pTransaction->ModificationsPtr();
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomByteArray<Key>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomByteArray<Key>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomByteArray<Key>() };

		// Act:
		test::PublishTransaction(*pPlugin, *pTransaction, sub);

		// Assert:
		ASSERT_EQ(0u, sub.numMatchingNotifications());
	}

	// endregion

	// region address interaction

	namespace {
		utils::KeySet ExtractAddedModificationKeys(const model::CosignatoryModification* pModifications, size_t modificationsCount) {
			utils::KeySet keys;
			for (auto i = 0u; i < modificationsCount; ++i) {
				if (CosignatoryModificationType::Add == pModifications[i].ModificationType)
					keys.insert(pModifications[i].CosignatoryPublicKey);
			}

			return keys;
		}

		std::vector<CosignatoryModificationType> GenerateRandomModificationTypeSequence(size_t numAdds, size_t numDels) {
			std::vector<CosignatoryModificationType> modificationTypes;
			for (auto i = 0u; i < numAdds + numDels; ++i)
				modificationTypes.push_back(i < numAdds ? CosignatoryModificationType::Add : CosignatoryModificationType::Del);

			std::mt19937 generator((std::random_device()()));
			std::shuffle(modificationTypes.begin(), modificationTypes.end(), generator);
			return modificationTypes;
		}

		template<typename TTraits>
		void AssertAddressInteractionNotifications(size_t numAddModifications, size_t numDelModifications) {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<AddressInteractionNotification> sub;
			auto pPlugin = TTraits::CreatePlugin();

			auto numModifications = static_cast<uint8_t>(numAddModifications + numDelModifications);
			auto pTransaction = CreateTransactionWithModifications<TTraits>(numModifications);
			test::FillWithRandomData(pTransaction->Signer);
			pTransaction->Type = static_cast<model::EntityType>(0x0815);
			auto modificationTypes = GenerateRandomModificationTypeSequence(numAddModifications, numDelModifications);
			auto* pModification = pTransaction->ModificationsPtr();
			for (auto modificationType : modificationTypes)
				*pModification++ = { modificationType, test::GenerateRandomByteArray<Key>() };

			// Act:
			test::PublishTransaction(*pPlugin, *pTransaction, sub);

			// Assert:
			ASSERT_EQ(0 < numAddModifications ? 1u : 0u, sub.numMatchingNotifications());
			if (0 < numAddModifications) {
				pModification = pTransaction->ModificationsPtr();
				auto addedKeys = ExtractAddedModificationKeys(pModification, numAddModifications + numDelModifications);
				const auto& notification = sub.matchingNotifications()[0];
				EXPECT_EQ(pTransaction->Signer, notification.Source);
				EXPECT_EQ(pTransaction->Type, notification.TransactionType);
				EXPECT_EQ(model::UnresolvedAddressSet(), notification.ParticipantsByAddress);
				EXPECT_EQ(addedKeys, notification.ParticipantsByKey);
			}
		}
	}

	PLUGIN_TEST(NoAddressInteractionNotificationWhenNoAddModificationsArePresent) {
		// Assert:
		AssertAddressInteractionNotifications<TTraits>(0, 0);
		AssertAddressInteractionNotifications<TTraits>(0, 3);
	}

	PLUGIN_TEST(CanPublishAddressInteractionNotificationWhenAddModificationsArePresent) {
		// Assert:
		AssertAddressInteractionNotifications<TTraits>(4, 0);
		AssertAddressInteractionNotifications<TTraits>(4, 3);
	}

	// endregion
}}
