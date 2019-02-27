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

#include "src/plugins/SecretProofTransactionPlugin.h"
#include "src/model/SecretLockNotifications.h"
#include "src/model/SecretProofTransaction.h"
#include "tests/test/SecretLockTransactionUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS SecretProofTransactionPluginTests

	// region TransactionPlugin

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(SecretProof, 1, 1)

		template<typename TTraits>
		auto CreateSecretProofTransaction() {
			return test::CreateRandomSecretProofTransaction<TTraits>(123);
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

		typename TTraits::TransactionType transaction;

		// Act:
		test::PublishTransaction(*pPlugin, transaction, sub);

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
		test::PublishTransaction(*pPlugin, *pTransaction, sub);

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
		test::PublishTransaction(*pPlugin, *pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		const auto& transaction = *pTransaction;
		EXPECT_EQ(transaction.HashAlgorithm, notification.HashAlgorithm);
		EXPECT_EQ(transaction.Secret, notification.Secret);
		ASSERT_EQ(transaction.ProofSize, notification.Proof.Size);
		EXPECT_EQ_MEMORY(transaction.ProofPtr(), notification.Proof.pData, notification.Proof.Size);
	}

	PLUGIN_TEST(CanSecretProofPublicationNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<ProofPublicationNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();
		auto pTransaction = CreateSecretProofTransaction<TTraits>();

		// Act:
		test::PublishTransaction(*pPlugin, *pTransaction, sub);

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
