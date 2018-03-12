#include "src/plugins/SecretProofTransactionPlugin.h"
#include "src/model/LockNotifications.h"
#include "src/model/SecretProofTransaction.h"
#include "tests/test/LockTransactionUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS SecretProofTransactionPluginTests

	// region TransactionPlugin

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(SecretProof);

		template<typename TTraits>
		auto CreateSecretProofTransaction() {
			return test::CreateSecretProofTransaction<TTraits>(123u);
		}
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Entity_Type_Secret_Proof)

	PLUGIN_TEST(CanCalculateSize) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin();
		typename TTraits::TransactionType transaction;
		transaction.ProofSize = 100;

		// Act:
		auto realSize = pPlugin->calculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 100, realSize);
	}

	// endregion

	// region accounts extraction

	PLUGIN_TEST(CanExtractAccounts) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction{};

		// Act:
		pPlugin->publish(transaction, sub);

		// Assert:
		EXPECT_EQ(3u, sub.numNotifications());
		EXPECT_EQ(0u, sub.numAddresses());
		EXPECT_EQ(0u, sub.numKeys());
	}

	// endregion

	// region lock hash algorithm notification

	PLUGIN_TEST(CanPublishHashAlgorithmNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<SecretLockHashAlgorithmNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();
		auto pTransaction = CreateSecretProofTransaction<TTraits>();

		// Act:
		pPlugin->publish(*pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(pTransaction->HashAlgorithm, notification.HashAlgorithm);
	}

	// endregion

	// region proof notification

	PLUGIN_TEST(CanSecretProofSecretNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<ProofSecretNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();
		auto pTransaction = CreateSecretProofTransaction<TTraits>();

		// Act:
		pPlugin->publish(*pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		const auto& transaction = *pTransaction;
		EXPECT_EQ(transaction.HashAlgorithm, notification.HashAlgorithm);
		EXPECT_EQ(transaction.Secret, notification.Secret);
		ASSERT_EQ(transaction.ProofSize, notification.Proof.Size);
		EXPECT_TRUE(0 == std::memcmp(transaction.ProofPtr(), notification.Proof.pData, notification.Proof.Size));
	}

	PLUGIN_TEST(CanSecretProofPublicationNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<ProofPublicationNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();
		auto pTransaction = CreateSecretProofTransaction<TTraits>();

		// Act:
		pPlugin->publish(*pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		const auto& transaction = *pTransaction;
		EXPECT_EQ(transaction.Signer, notification.Signer);
		EXPECT_EQ(transaction.HashAlgorithm, notification.HashAlgorithm);
		EXPECT_EQ(transaction.Secret, notification.Secret);
	}

	// endregion
}}
