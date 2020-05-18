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

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(SecretProof, 1, 1,)
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Secret_Proof)

	// endregion

	// region publish

	PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrder) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			SecretLockHashAlgorithmNotification::Notification_Type,
			ProofSecretNotification::Notification_Type,
			ProofPublicationNotification::Notification_Type
		});
	}

	PLUGIN_TEST(CanPublishAllNotifications) {
		// Arrange:
		auto pTransaction = test::CreateRandomSecretProofTransaction<TTraits>(123);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		builder.template addExpectation<SecretLockHashAlgorithmNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.HashAlgorithm, notification.HashAlgorithm);
		});
		builder.template addExpectation<ProofSecretNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.HashAlgorithm, notification.HashAlgorithm);
			EXPECT_EQ(transaction.Secret, notification.Secret);
			ASSERT_EQ(transaction.ProofSize, notification.Proof.Size);
			EXPECT_EQ_MEMORY(transaction.ProofPtr(), notification.Proof.pData, notification.Proof.Size);
		});
		builder.template addExpectation<ProofPublicationNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.SignerPublicKey, notification.Owner);
			EXPECT_EQ(transaction.HashAlgorithm, notification.HashAlgorithm);
			EXPECT_EQ(transaction.Secret, notification.Secret);
			EXPECT_EQ(transaction.RecipientAddress, notification.Recipient);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion
}}
