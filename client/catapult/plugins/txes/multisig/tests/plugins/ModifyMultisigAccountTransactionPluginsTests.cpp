#include "src/plugins/ModifyMultisigAccountTransactionPlugins.h"
#include "src/model/ModifyMultisigAccountTransaction.h"
#include "src/model/MultisigNotifications.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS ModifyMultisigAccountTransactionPluginsTests

	// region TransactionPlugin

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(ModifyMultisigAccount);

		template<typename TTraits>
		auto CreateTransactionWithModifications(uint8_t numModifications) {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + numModifications * sizeof(CosignatoryModification);
			std::unique_ptr<TransactionType> pTransaction(reinterpret_cast<TransactionType*>(::operator new(entitySize)));
			pTransaction->Size = entitySize;
			pTransaction->ModificationsCount = numModifications;
			test::FillWithRandomData(pTransaction->Signer);
			return pTransaction;
		}
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, EntityType::Modify_Multisig_Account)

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

	// region publish

	// region basic

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
			pPlugin->publish(transaction, sub);

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
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomData<Key_Size>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomData<Key_Size>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomData<Key_Size>() };

		// Assert:
		AssertNumNotifications<TTraits>(2, *pTransaction);
	}

	PLUGIN_TEST(CanPublishCorrectNumberOfNotificationsWhenAddModificationsArePresent) {
		// Arrange:
		auto pTransaction = CreateTransactionWithModifications<TTraits>(3);
		auto* pModification = pTransaction->ModificationsPtr();
		*pModification++ = { CosignatoryModificationType::Add, test::GenerateRandomData<Key_Size>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomData<Key_Size>() };
		*pModification++ = { CosignatoryModificationType::Add, test::GenerateRandomData<Key_Size>() };

		// Assert:
		AssertNumNotifications<TTraits>(4, *pTransaction, [](const auto& sub) {
			// - multisig modify new cosigner notifications must be the first raised notifications
			EXPECT_EQ(Multisig_Modify_New_Cosigner_Notification, sub.notificationTypes()[0]);
			EXPECT_EQ(Multisig_Modify_New_Cosigner_Notification, sub.notificationTypes()[1]);
		});
	}

	// endregion

	// region modify multisig settings notification

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
		pPlugin->publish(transaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(transaction.Signer, notification.Signer);
		EXPECT_EQ(-3, notification.MinRemovalDelta);
		EXPECT_EQ(4, notification.MinApprovalDelta);
	}

	// endregion

	// region modify multisig cosigners notification

	PLUGIN_TEST(CanPublishCosignersNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<ModifyMultisigCosignersNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithModifications<TTraits>(3);
		auto* pModification = pTransaction->ModificationsPtr();
		*pModification++ = { CosignatoryModificationType::Add, test::GenerateRandomData<Key_Size>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomData<Key_Size>() };
		*pModification++ = { CosignatoryModificationType::Add, test::GenerateRandomData<Key_Size>() };

		// Act:
		pPlugin->publish(*pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(pTransaction->Signer, notification.Signer);
		EXPECT_EQ(3, notification.ModificationsCount);
		EXPECT_EQ(pTransaction->ModificationsPtr(), notification.ModificationsPtr);
	}

	PLUGIN_TEST(NoCosignersNotificationIfNoModificationIsPresent) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<ModifyMultisigCosignersNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		transaction.ModificationsCount = 0;

		// Act:
		pPlugin->publish(transaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numNotifications());
		ASSERT_EQ(0u, sub.numMatchingNotifications());
	}

	// endregion

	// region modify multisig new cosigner notification

	PLUGIN_TEST(CanPublishNewCosignerNotificationForEachAddModification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<ModifyMultisigNewCosignerNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithModifications<TTraits>(3);
		auto* pModification = pTransaction->ModificationsPtr();
		*pModification++ = { CosignatoryModificationType::Add, test::GenerateRandomData<Key_Size>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomData<Key_Size>() };
		*pModification++ = { CosignatoryModificationType::Add, test::GenerateRandomData<Key_Size>() };

		// Act:
		pPlugin->publish(*pTransaction, sub);

		// Assert:
		ASSERT_EQ(2u, sub.numMatchingNotifications());

		EXPECT_EQ(pTransaction->Signer, sub.matchingNotifications()[0].MultisigAccountKey);
		EXPECT_EQ(pTransaction->ModificationsPtr()[0].CosignatoryPublicKey, sub.matchingNotifications()[0].CosignatoryKey);

		EXPECT_EQ(pTransaction->Signer, sub.matchingNotifications()[1].MultisigAccountKey);
		EXPECT_EQ(pTransaction->ModificationsPtr()[2].CosignatoryPublicKey, sub.matchingNotifications()[1].CosignatoryKey);
	}

	PLUGIN_TEST(NoNewCosignerNotificationsIfNoAddModificationsArePresent) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<ModifyMultisigNewCosignerNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithModifications<TTraits>(3);
		auto* pModification = pTransaction->ModificationsPtr();
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomData<Key_Size>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomData<Key_Size>() };
		*pModification++ = { CosignatoryModificationType::Del, test::GenerateRandomData<Key_Size>() };

		// Act:
		pPlugin->publish(*pTransaction, sub);

		// Assert:
		ASSERT_EQ(0u, sub.numMatchingNotifications());
	}

	// endregion

	// endregion
}}
