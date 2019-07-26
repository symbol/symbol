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
		EXPECT_EQ(1u, attributes.MinVersion);
		EXPECT_EQ(1u, attributes.MaxVersion);

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
		EXPECT_EQ(1u, attributes.MinVersion);
		EXPECT_EQ(1u, attributes.MaxVersion);

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

	// region publish - neither sub transactions nor signatures

	namespace {
		struct AggregateTransactionTraits {
		public:
			using TransactionType = AggregateTransaction;

		public:
			static auto CreatePlugin(const TransactionRegistry& transactionRegistry) {
				return CreateAggregateTransactionPlugin(transactionRegistry, Entity_Type);
			}
		};
	}

	TEST(TEST_CLASS, CanPublishAllNotificationsInCorrectOrderWhenNeitherSubTransactionsNorCosignaturesArePresent) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto wrapper = CreateAggregateTransaction(0, 0);

		// Act + Assert:
		test::TransactionPluginTestUtils<AggregateTransactionTraits>::AssertNotificationTypes(*wrapper.pTransaction, {
			// aggregate cosignatures notification must be the first raised notification
			AggregateCosignaturesNotification::Notification_Type
		}, registry);
	}

	TEST(TEST_CLASS, CanPublishAllNotificationsWhenNeitherSubTransactionsNorCosignaturesArePresent) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto wrapper = CreateAggregateTransaction(0, 0);

		const auto& transaction = *wrapper.pTransaction;
		test::TransactionPluginTestUtils<AggregateTransactionTraits>::PublishTestBuilder builder;
		builder.addExpectation<AggregateCosignaturesNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Signer);
			EXPECT_EQ(0u, notification.TransactionsCount);
			EXPECT_FALSE(!!notification.TransactionsPtr);
			EXPECT_EQ(0u, notification.CosignaturesCount);
			EXPECT_FALSE(!!notification.CosignaturesPtr);
		});

		// Act + Assert:
		builder.runTest(transaction, registry);
	}

	// endregion

	// region publish - only sub transactions

	namespace {
		void AddSubTransactionExpectations(
				test::TransactionPluginTestUtils<AggregateTransactionTraits>::PublishTestBuilder& builder,
				const AggregateTransactionWrapper& wrapper,
				uint8_t count) {
			for (auto i = 0u; i < count; ++i) {
				builder.addExpectation<SourceChangeNotification>(i, [](const auto& notification) {
					EXPECT_EQ(0u, notification.PrimaryId);
					EXPECT_EQ(1u, notification.SecondaryId);
					EXPECT_EQ(SourceChangeNotification::SourceChangeType::Relative, notification.PrimaryChangeType);
					EXPECT_EQ(SourceChangeNotification::SourceChangeType::Relative, notification.SecondaryChangeType);
				});
				builder.addExpectation<AccountPublicKeyNotification>(i * 2, [&wrapper, i](const auto& notification) {
					EXPECT_EQ(wrapper.SubTransactions[i]->Signer, notification.PublicKey);
				});
				builder.addExpectation<EntityNotification>(i, [i](const auto& notification) {
					// min/max version comes from MockTransactionPlugin created in CreateDefaultTransactionRegistry
					EXPECT_EQ(static_cast<NetworkIdentifier>(100 + i), notification.NetworkIdentifier);
					EXPECT_EQ((i + 1) * 2, notification.EntityVersion);
					EXPECT_EQ(0x02u, notification.MinVersion);
					EXPECT_EQ(0xFEu, notification.MaxVersion);
				});
				builder.addExpectation<AggregateEmbeddedTransactionNotification>(i, [&wrapper, i](const auto& notification) {
					EXPECT_EQ(wrapper.pTransaction->Signer, notification.Signer);
					EXPECT_EQ(*wrapper.SubTransactions[i], notification.Transaction);
					EXPECT_EQ(wrapper.pTransaction->CosignaturesCount(), notification.CosignaturesCount);
					EXPECT_EQ(wrapper.pTransaction->CosignaturesPtr(), notification.CosignaturesPtr);
				});
				builder.addExpectation<AccountPublicKeyNotification>(i * 2 + 1, [&wrapper, i](const auto& notification) {
					EXPECT_EQ(wrapper.SubTransactions[i]->Recipient, notification.PublicKey);
				});
			}
		}
	}

	TEST(TEST_CLASS, CanPublishAllNotificationsInCorrectOrderWhenOnlySubTransactionsArePresent) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto wrapper = CreateAggregateTransaction(2, 0);

		// Act + Assert:
		test::TransactionPluginTestUtils<AggregateTransactionTraits>::AssertNotificationTypes(*wrapper.pTransaction, {
			// aggregate cosignatures notification must be the first raised notification
			AggregateCosignaturesNotification::Notification_Type,

			// source change notification must be the first raised sub-transaction notification
			SourceChangeNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			EntityNotification::Notification_Type,
			AggregateEmbeddedTransactionNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,

			SourceChangeNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			EntityNotification::Notification_Type,
			AggregateEmbeddedTransactionNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
		}, registry);
	}

	TEST(TEST_CLASS, CanPublishAllNotificationsWhenOnlySubTransactionsArePresent) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto wrapper = CreateAggregateTransaction(2, 0);

		const auto& transaction = *wrapper.pTransaction;
		test::TransactionPluginTestUtils<AggregateTransactionTraits>::PublishTestBuilder builder;
		builder.addExpectation<AggregateCosignaturesNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Signer);
			EXPECT_EQ(2u, notification.TransactionsCount);
			EXPECT_EQ(transaction.TransactionsPtr(), notification.TransactionsPtr);
			EXPECT_EQ(0u, notification.CosignaturesCount);
			EXPECT_FALSE(!!notification.CosignaturesPtr);
		});

		AddSubTransactionExpectations(builder, wrapper, 2);

		// Act + Assert:
		builder.runTest(transaction, registry);
	}

	// endregion

	// region publish - only cosignatures

	namespace {
		void AddCosignatureExpectations(
				test::TransactionPluginTestUtils<AggregateTransactionTraits>::PublishTestBuilder& builder,
				const AggregateTransactionWrapper& wrapper,
				const Hash256& aggregateDataHash,
				uint8_t count) {
			for (auto i = 0u; i < count; ++i) {
				builder.addExpectation<SignatureNotification>(i, [&wrapper, &aggregateDataHash, i](const auto& notification) {
					// notifications should refer to cosigners
					EXPECT_EQ(wrapper.Cosigners[i], notification.Signer);
					EXPECT_EQ(wrapper.CosignerSignatures[i], notification.Signature);

					// notifications should refer to same (aggregate) data hash
					EXPECT_EQ(aggregateDataHash.data(), notification.Data.pData);
					EXPECT_EQ(Hash256::Size, notification.Data.Size);

					// notifications should not have replay protection because they represent cosignatures
					EXPECT_EQ(SignatureNotification::ReplayProtectionMode::Disabled, notification.DataReplayProtectionMode);
				});
			}
		}
	}

	TEST(TEST_CLASS, CanPublishAllNotificationsInCorrectOrderWhenOnlyCosignaturesArePresent) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto wrapper = CreateAggregateTransaction(0, 3);

		// Act + Assert:
		test::TransactionPluginTestUtils<AggregateTransactionTraits>::AssertNotificationTypes(*wrapper.pTransaction, {
			// aggregate cosignatures notification must be the first raised notification
			AggregateCosignaturesNotification::Notification_Type,

			// signature notifications are raised last (and with wrong source) for performance reasons
			SignatureNotification::Notification_Type,
			SignatureNotification::Notification_Type,
			SignatureNotification::Notification_Type
		}, registry);
	}

	TEST(TEST_CLASS, CanPublishAllNotificationsWhenOnlyCosignaturesArePresent) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto wrapper = CreateAggregateTransaction(0, 3);
		auto aggregateDataHash = test::GenerateRandomByteArray<Hash256>();

		const auto& transaction = *wrapper.pTransaction;
		test::TransactionPluginTestUtils<AggregateTransactionTraits>::PublishTestBuilder builder;
		builder.addExpectation<AggregateCosignaturesNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Signer);
			EXPECT_EQ(0u, notification.TransactionsCount);
			EXPECT_FALSE(!!notification.TransactionsPtr);
			EXPECT_EQ(3u, notification.CosignaturesCount);
			EXPECT_EQ(transaction.CosignaturesPtr(), notification.CosignaturesPtr);
		});

		AddCosignatureExpectations(builder, wrapper, aggregateDataHash, 3);

		// Act + Assert:
		builder.runTestWithHash(transaction, aggregateDataHash, registry);
	}

	// endregion

	// region sub transactions and cosignatures

	TEST(TEST_CLASS, CanPublishAllNotificationsInCorrectOrderWhenSubTransactionsAndCosignaturesArePresent) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto wrapper = CreateAggregateTransaction(2, 3);

		// Act + Assert:
		test::TransactionPluginTestUtils<AggregateTransactionTraits>::AssertNotificationTypes(*wrapper.pTransaction, {
			// aggregate cosignatures notification must be the first raised notification
			AggregateCosignaturesNotification::Notification_Type,

			// source change notification must be the first raised sub-transaction notification
			SourceChangeNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			EntityNotification::Notification_Type,
			AggregateEmbeddedTransactionNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,

			SourceChangeNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			EntityNotification::Notification_Type,
			AggregateEmbeddedTransactionNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,

			// signature notifications are raised last (and with wrong source) for performance reasons
			SignatureNotification::Notification_Type,
			SignatureNotification::Notification_Type,
			SignatureNotification::Notification_Type
		}, registry);
	}

	TEST(TEST_CLASS, CanPublishAllNotificationsWhenSubTransactionsAndCosignaturesArePresent) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto wrapper = CreateAggregateTransaction(2, 3);
		auto aggregateDataHash = test::GenerateRandomByteArray<Hash256>();

		const auto& transaction = *wrapper.pTransaction;
		test::TransactionPluginTestUtils<AggregateTransactionTraits>::PublishTestBuilder builder;
		builder.addExpectation<AggregateCosignaturesNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.Signer, notification.Signer);
			EXPECT_EQ(2u, notification.TransactionsCount);
			EXPECT_EQ(transaction.TransactionsPtr(), notification.TransactionsPtr);
			EXPECT_EQ(3u, notification.CosignaturesCount);
			EXPECT_EQ(transaction.CosignaturesPtr(), notification.CosignaturesPtr);
		});

		AddSubTransactionExpectations(builder, wrapper, 2);
		AddCosignatureExpectations(builder, wrapper, aggregateDataHash, 3);

		// Act + Assert:
		builder.runTestWithHash(transaction, aggregateDataHash, registry);
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
		AssertCanExtractDataBufferFromAggregate(0, 0);
	}

	TEST(TEST_CLASS, CanExtractDataBufferFromAggregate) {
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
				EXPECT_EQ(Key::Size, buffers[i].Size) << message;
				++pCosignature;
			}
		}
	}

	TEST(TEST_CLASS, CanExtractMerkleSupplementaryBuffersFromEmptyAggregate) {
		AssertCanExtractMerkleSupplementaryBuffersFromAggregate(0, 0);
	}

	TEST(TEST_CLASS, CanExtractMerkleSupplementaryBuffersFromAggregate) {
		AssertCanExtractMerkleSupplementaryBuffersFromAggregate(2, 3);
	}

	// endregion
}}
