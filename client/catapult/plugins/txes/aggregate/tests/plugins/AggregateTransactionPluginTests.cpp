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

#include "src/plugins/AggregateTransactionPlugin.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "src/model/AggregateNotifications.h"
#include "src/model/AggregateTransaction.h"
#include "catapult/model/Address.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS AggregateTransactionPluginTests

	// region AggregateTransactionWrapper

	namespace {
		constexpr auto Entity_Type = static_cast<EntityType>(9876);

		struct AggregateTransactionWrapper {
			std::unique_ptr<AggregateTransaction> pTransaction;
			std::vector<const mocks::EmbeddedMockTransaction*> SubTransactions;
			std::vector<Key> SubTransactionSigners;
			std::vector<Key> SubTransactionRecipients;
			std::vector<Key> Cosigners;
			std::vector<Signature> CosignerSignatures;
		};

		AggregateTransactionWrapper CreateAggregateTransaction(uint8_t numTransactions, uint8_t numCosignatures) {
			using TransactionType = AggregateTransaction;
			uint32_t entitySize = sizeof(TransactionType)
					+ numTransactions * sizeof(mocks::EmbeddedMockTransaction)
					+ numCosignatures * sizeof(Cosignature);

			AggregateTransactionWrapper wrapper;
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			pTransaction->Size = entitySize;
			pTransaction->PayloadSize = numTransactions * sizeof(mocks::EmbeddedMockTransaction);
			test::FillWithRandomData(pTransaction->Signer);
			test::FillWithRandomData(pTransaction->Signature);

			auto* pSubTransaction = static_cast<mocks::EmbeddedMockTransaction*>(pTransaction->TransactionsPtr());
			for (uint8_t i = 0; i < numTransactions; ++i) {
				pSubTransaction->Size = sizeof(mocks::EmbeddedMockTransaction);
				pSubTransaction->Data.Size = 0;
				pSubTransaction->Type = mocks::EmbeddedMockTransaction::Entity_Type;
				pSubTransaction->Version = model::MakeVersion(static_cast<model::NetworkIdentifier>(100 + i), (i + 1) * 2);
				test::FillWithRandomData(pSubTransaction->Signer);
				test::FillWithRandomData(pSubTransaction->Recipient);

				wrapper.SubTransactions.push_back(pSubTransaction);
				wrapper.SubTransactionSigners.push_back(pSubTransaction->Signer);
				wrapper.SubTransactionRecipients.push_back(pSubTransaction->Recipient);
				++pSubTransaction;
			}

			auto* pCosignature = pTransaction->CosignaturesPtr();
			for (auto i = 0u; i < numCosignatures; ++i) {
				test::FillWithRandomData(pCosignature->Signer);
				test::FillWithRandomData(pCosignature->Signature);

				wrapper.Cosigners.push_back(pCosignature->Signer);
				wrapper.CosignerSignatures.push_back(pCosignature->Signature);
				++pCosignature;
			}

			wrapper.pTransaction = std::move(pTransaction);
			return wrapper;
		}
	}

	// endregion

	// region basic

	TEST(TEST_CLASS, CanCreatePlugin) {
		// Act:
		TransactionRegistry registry;
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);

		// Assert:
		EXPECT_EQ(Entity_Type, pPlugin->type());
	}

	TEST(TEST_CLASS, PluginSupportsTopLevel) {
		// Arrange:
		TransactionRegistry registry;
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);

		// Act + Assert:
		EXPECT_TRUE(pPlugin->supportsTopLevel());
	}

	TEST(TEST_CLASS, PluginDoesNotSupportEmbedding) {
		// Arrange:
		TransactionRegistry registry;
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);

		// Act + Assert:
		EXPECT_FALSE(pPlugin->supportsEmbedding());
		EXPECT_THROW(pPlugin->embeddedPlugin(), catapult_runtime_error);
	}

	// endregion

	// region attributes

	TEST(TEST_CLASS, AttributesReturnsCorrectValuesWhenConfiguredWithDefaultMaxLifetime) {
		// Arrange:
		TransactionRegistry registry;
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);

		// Act:
		auto attributes = pPlugin->attributes();

		// Assert:
		EXPECT_EQ(2u, attributes.MinVersion);
		EXPECT_EQ(2u, attributes.MaxVersion);

		// - zero denotes default lifetime should be used
		EXPECT_EQ(utils::TimeSpan(), attributes.MaxLifetime);
	}

	TEST(TEST_CLASS, AttributesReturnsCorrectValuesWhenConfiguredWithCustomMaxLifetime) {
		// Arrange:
		TransactionRegistry registry;
		auto pPlugin = CreateAggregateTransactionPlugin(registry, utils::TimeSpan::FromMinutes(1234), Entity_Type);

		// Act:
		auto attributes = pPlugin->attributes();

		// Assert:
		EXPECT_EQ(2u, attributes.MinVersion);
		EXPECT_EQ(2u, attributes.MaxVersion);

		EXPECT_EQ(utils::TimeSpan::FromMinutes(1234), attributes.MaxLifetime);
	}

	// endregion

	// region size

	TEST(TEST_CLASS, CanCalculateSizeWhenAllSubTransactionsAreSupported) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
		auto wrapper = CreateAggregateTransaction(3, 4);

		// Act:
		auto realSize = pPlugin->calculateRealSize(*wrapper.pTransaction);

		// Assert:
		EXPECT_EQ(sizeof(AggregateTransaction) + 3 * sizeof(mocks::EmbeddedMockTransaction) + 4 * sizeof(Cosignature), realSize);
	}

	TEST(TEST_CLASS, CannotCalculateSizeWhenAnySubTransactionIsNotSupported) {
		// Arrange:
		TransactionRegistry registry;
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
		auto wrapper = CreateAggregateTransaction(3, 4);

		// Act:
		auto realSize = pPlugin->calculateRealSize(*wrapper.pTransaction);

		// Assert:
		EXPECT_EQ(std::numeric_limits<uint64_t>::max(), realSize);
	}

	// endregion

	// region publish - basic

	TEST(TEST_CLASS, CanRaiseCorrectNumberOfNotificationsFromEmptyAggregate) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
		auto wrapper = CreateAggregateTransaction(0, 0);

		// Act:
		test::PublishTransaction(*pPlugin, *wrapper.pTransaction, sub);

		// Assert:
		// - 1 AggregateCosignaturesNotification
		ASSERT_EQ(1u, sub.numNotifications());

		// - aggregate cosignatures notification must be the first raised notification
		EXPECT_EQ(Aggregate_Cosignatures_Notification, sub.notificationTypes()[0]);
	}

	TEST(TEST_CLASS, CanRaiseCorrectNumberOfNotificationsFromAggregate) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
		auto wrapper = CreateAggregateTransaction(2, 3);

		// Act:
		test::PublishTransaction(*pPlugin, *wrapper.pTransaction, sub);

		// Assert:
		// - 1 AggregateCosignaturesNotification
		// - 3 SignatureNotification (one per cosigner)
		// - 2 SourceChangeNotification (one per embedded-mock)
		// - 4 AccountPublicKeyNotification (two per embedded-mock; one signer and one recipient each)
		// - 2 EntityNotification (one per embedded-mock)
		// - 2 AggregateEmbeddedTransactionNotification (one per embedded-mock)
		ASSERT_EQ(1u + 3 + 2 + 4 + 2 + 2, sub.numNotifications());

		// - aggregate cosignatures notification must be the first raised notification
		EXPECT_EQ(Aggregate_Cosignatures_Notification, sub.notificationTypes()[0]);

		// - source change notification must be the first raised sub-transaction notification
		for (auto i = 0u; i < 2u; ++i) {
			auto offset = i * 5;
			auto message = "sub-transaction at " + std::to_string(i);
			EXPECT_EQ(Core_Source_Change_Notification, sub.notificationTypes()[offset + 1]) << message;
			EXPECT_EQ(Core_Register_Account_Public_Key_Notification, sub.notificationTypes()[offset + 2]) << message;
			EXPECT_EQ(Core_Entity_Notification, sub.notificationTypes()[offset + 3]) << message;
			EXPECT_EQ(Aggregate_EmbeddedTransaction_Notification, sub.notificationTypes()[offset + 4]) << message;
			EXPECT_EQ(Core_Register_Account_Public_Key_Notification, sub.notificationTypes()[offset + 5]) << message;
		}

		// - signature notifications are raised last (and with wrong source) for performance reasons
		EXPECT_EQ(Core_Signature_Notification, sub.notificationTypes()[11]);
		EXPECT_EQ(Core_Signature_Notification, sub.notificationTypes()[12]);
		EXPECT_EQ(Core_Signature_Notification, sub.notificationTypes()[13]);
	}

	// endregion

	// region publish - account (including nested)

	namespace {
		void AssertContainsAllKeys(const mocks::MockNotificationSubscriber& sub, const AggregateTransactionWrapper& wrapper) {
			auto i = 0u;
			for (const auto& key : wrapper.SubTransactionSigners) {
				EXPECT_TRUE(sub.contains(key)) << "sub transaction signer " << i;
				++i;
			}

			for (const auto& key : wrapper.SubTransactionRecipients) {
				EXPECT_TRUE(sub.contains(key)) << "sub transaction recipient " << i;
				++i;
			}

			// Sanity:
			EXPECT_EQ(i, sub.numKeys());
		}
	}

	TEST(TEST_CLASS, CanRaiseAccountNotificationsFromAggregate) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
		auto wrapper = CreateAggregateTransaction(2, 3);

		// Act:
		test::PublishTransaction(*pPlugin, *wrapper.pTransaction, sub);

		// Assert: 2 sub signer and 2 sub recipient notifications are raised
		EXPECT_EQ(0u, sub.numAddresses());
		EXPECT_EQ(4u, sub.numKeys());
		AssertContainsAllKeys(sub, wrapper);
	}

	// endregion

	// region publish - source change notification

	TEST(TEST_CLASS, CanRaiseSourceChangeNotificationsFromAggregate) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<SourceChangeNotification> sub;
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
		auto wrapper = CreateAggregateTransaction(2, 3);

		// Act:
		test::PublishTransaction(*pPlugin, *wrapper.pTransaction, sub);

		// Assert: one notification is raised for each embedded transaction
		ASSERT_EQ(2u, sub.numMatchingNotifications());
		for (auto i = 0u; i < 2; ++i) {
			auto message = "transaction at " + std::to_string(i);
			const auto& notification = sub.matchingNotifications()[i];

			EXPECT_EQ(0u, notification.PrimaryId) << message;
			EXPECT_EQ(1u, notification.SecondaryId) << message;
			EXPECT_EQ(SourceChangeNotification::SourceChangeType::Relative, notification.PrimaryChangeType) << message;
			EXPECT_EQ(SourceChangeNotification::SourceChangeType::Relative, notification.SecondaryChangeType) << message;
		}
	}

	// endregion

	// region publish - entity notification

	TEST(TEST_CLASS, CanRaiseEntityNotificationsFromAggregate) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<EntityNotification> sub;
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
		auto wrapper = CreateAggregateTransaction(2, 3);

		// Act:
		test::PublishTransaction(*pPlugin, *wrapper.pTransaction, sub);

		// Assert: one notification is raised for each embedded transaction
		ASSERT_EQ(2u, sub.numMatchingNotifications());
		for (auto i = 0u; i < 2; ++i) {
			auto message = "transaction at " + std::to_string(i);
			const auto& notification = sub.matchingNotifications()[i];

			// - min/max version comes from MockTransactionPlugin created in CreateDefaultTransactionRegistry
			EXPECT_EQ(static_cast<NetworkIdentifier>(100 + i), notification.NetworkIdentifier) << message;
			EXPECT_EQ((i + 1) * 2, notification.EntityVersion) << message;
			EXPECT_EQ(0x02u, notification.MinVersion) << message;
			EXPECT_EQ(0xFEu, notification.MaxVersion) << message;
		}
	}

	// endregion

	// region publish - aggregate embedded transaction

	namespace {
		void AssertCanRaiseEmbeddedTransactionNotifications(uint8_t numTransactions, uint8_t numCosignatures) {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<AggregateEmbeddedTransactionNotification> sub;
			auto registry = mocks::CreateDefaultTransactionRegistry();
			auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
			auto wrapper = CreateAggregateTransaction(numTransactions, numCosignatures);

			// Act:
			test::PublishTransaction(*pPlugin, *wrapper.pTransaction, sub);

			// Assert: the plugin raises an embedded transaction notification for each transaction
			ASSERT_EQ(numTransactions, sub.numMatchingNotifications());
			for (auto i = 0u; i < numTransactions; ++i) {
				auto message = "transaction at " + std::to_string(i);
				const auto& notification = sub.matchingNotifications()[i];

				EXPECT_EQ(wrapper.pTransaction->Signer, notification.Signer) << message;
				EXPECT_EQ(*wrapper.SubTransactions[i], notification.Transaction) << message;
				EXPECT_EQ(numCosignatures, notification.CosignaturesCount) << message;
				EXPECT_EQ(wrapper.pTransaction->CosignaturesPtr(), notification.CosignaturesPtr) << message;
			}
		}
	}

	TEST(TEST_CLASS, EmptyAggregateDoesNotRaiseEmbeddedTransactionNotifications) {
		// Assert:
		AssertCanRaiseEmbeddedTransactionNotifications(0, 0);
	}

	TEST(TEST_CLASS, CanRaiseEmbeddedTransactionNotificationsFromAggregate) {
		// Assert:
		AssertCanRaiseEmbeddedTransactionNotifications(2, 3);
	}

	// endregion

	// region publish - signature

	namespace {
		void AssertCanRaiseSignatureNotifications(uint8_t numTransactions, uint8_t numCosignatures) {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<SignatureNotification> sub;
			auto registry = mocks::CreateDefaultTransactionRegistry();
			auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
			auto wrapper = CreateAggregateTransaction(numTransactions, numCosignatures);

			auto aggregateDataHash = test::GenerateRandomByteArray<Hash256>();

			// Act:
			test::PublishTransaction(*pPlugin, WeakEntityInfoT<Transaction>(*wrapper.pTransaction, aggregateDataHash), sub);

			// Assert: the plugin only raises signature notifications for explicit cosigners
			//         (the signature notification for the signer is raised as part of normal processing)
			ASSERT_EQ(numCosignatures, sub.numMatchingNotifications());
			for (auto i = 0u; i < numCosignatures; ++i) {
				auto message = "cosigner at " + std::to_string(i);
				const auto& notification = sub.matchingNotifications()[i];

				// - notifications should refer to cosigners
				EXPECT_EQ(wrapper.Cosigners[i], notification.Signer) << message;
				EXPECT_EQ(wrapper.CosignerSignatures[i], notification.Signature) << message;

				// - notifications should refer to same (aggregate) data hash
				EXPECT_EQ(aggregateDataHash.data(), notification.Data.pData) << message;
				EXPECT_EQ(Hash256_Size, notification.Data.Size) << message;

				// - notifications should not have replay protection because they represent cosignatures
				EXPECT_EQ(SignatureNotification::ReplayProtectionMode::Disabled, notification.DataReplayProtectionMode);
			}
		}
	}

	TEST(TEST_CLASS, EmptyAggregateDoesNotRaiseSignatureNotifications) {
		// Assert:
		AssertCanRaiseSignatureNotifications(0, 0);
	}

	TEST(TEST_CLASS, CanRaiseSignatureNotificationsFromAggregate) {
		// Assert:
		AssertCanRaiseSignatureNotifications(2, 3);
	}

	// endregion

	// region publish - aggregate cosignatures

	TEST(TEST_CLASS, CanRaiseAggregateCosignaturesNotificationsFromEmptyAggregate) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<AggregateCosignaturesNotification> sub;
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
		auto wrapper = CreateAggregateTransaction(0, 0);

		// Act:
		test::PublishTransaction(*pPlugin, *wrapper.pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(wrapper.pTransaction->Signer, notification.Signer);
		EXPECT_EQ(0u, notification.TransactionsCount);
		EXPECT_FALSE(!!notification.TransactionsPtr);
		EXPECT_EQ(0u, notification.CosignaturesCount);
		EXPECT_FALSE(!!notification.CosignaturesPtr);
	}

	TEST(TEST_CLASS, CanRaiseAggregateCosignaturesNotificationsFromAggregate) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<AggregateCosignaturesNotification> sub;
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
		auto wrapper = CreateAggregateTransaction(2, 3);

		// Act:
		test::PublishTransaction(*pPlugin, *wrapper.pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(wrapper.pTransaction->Signer, notification.Signer);
		EXPECT_EQ(2u, notification.TransactionsCount);
		EXPECT_EQ(wrapper.pTransaction->TransactionsPtr(), notification.TransactionsPtr);
		EXPECT_EQ(3u, notification.CosignaturesCount);
		EXPECT_EQ(wrapper.pTransaction->CosignaturesPtr(), notification.CosignaturesPtr);
	}

	// endregion

	// region dataBuffer

	namespace {
		void AssertCanExtractDataBufferFromAggregate(uint8_t numTransactions, uint8_t numCosignatures) {
			// Arrange:
			auto registry = mocks::CreateDefaultTransactionRegistry();
			auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
			auto wrapper = CreateAggregateTransaction(numTransactions, numCosignatures);

			const auto* pAggregateDataStart = test::AsVoidPointer(&wrapper.pTransaction->Version);
			auto aggregateDataSize = sizeof(AggregateTransaction) - VerifiableEntity::Header_Size;
			aggregateDataSize += numTransactions * sizeof(mocks::EmbeddedMockTransaction);

			// Act:
			auto buffer = pPlugin->dataBuffer(*wrapper.pTransaction);

			// Assert:
			EXPECT_EQ(pAggregateDataStart, buffer.pData);
			ASSERT_EQ(aggregateDataSize, buffer.Size);
		}
	}

	TEST(TEST_CLASS, CanExtractDataBufferFromEmptyAggregate) {
		// Assert:
		AssertCanExtractDataBufferFromAggregate(0, 0);
	}

	TEST(TEST_CLASS, CanExtractDataBufferFromNonEmptyAggregate) {
		// Assert:
		AssertCanExtractDataBufferFromAggregate(2, 3);
	}

	// endregion

	// region merkleSupplementaryBuffers

	namespace {
		void AssertCanExtractMerkleSupplementaryBuffersFromAggregate(uint8_t numTransactions, uint8_t numCosignatures) {
			// Arrange:
			auto registry = mocks::CreateDefaultTransactionRegistry();
			auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
			auto wrapper = CreateAggregateTransaction(numTransactions, numCosignatures);

			// Act:
			auto buffers = pPlugin->merkleSupplementaryBuffers(*wrapper.pTransaction);

			// Assert:
			ASSERT_EQ(numCosignatures, buffers.size());

			const auto* pCosignature = wrapper.pTransaction->CosignaturesPtr();
			for (auto i = 0u; i < numCosignatures; ++i) {
				auto message = "buffer " + std::to_string(i);
				EXPECT_EQ(pCosignature->Signer.data(), buffers[i].pData) << message;
				EXPECT_EQ(Key_Size, buffers[i].Size) << message;
				++pCosignature;
			}
		}
	}

	TEST(TEST_CLASS, CanExtractMerkleSupplementaryBuffersFromEmptyAggregate) {
		// Assert:
		AssertCanExtractMerkleSupplementaryBuffersFromAggregate(0, 0);
	}

	TEST(TEST_CLASS, CanExtractMerkleSupplementaryBuffersFromNonEmptyAggregate) {
		// Assert:
		AssertCanExtractMerkleSupplementaryBuffersFromAggregate(2, 3);
	}

	// endregion
}}
