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
#include "catapult/utils/IntegerMath.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
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
			std::vector<Cosignature> Cosignatures;
		};

		AggregateTransactionWrapper CreateAggregateTransaction(uint8_t numTransactions, uint8_t numCosignatures) {
			using TransactionType = AggregateTransaction;

			uint32_t transactionSize = sizeof(mocks::EmbeddedMockTransaction);
			uint32_t txPaddingSize = utils::GetPaddingSize(transactionSize, 8);
			uint32_t payloadSize = numTransactions * (transactionSize + txPaddingSize);
			uint32_t entitySize = SizeOf32<TransactionType>() + payloadSize + numCosignatures * SizeOf32<Cosignature>();

			AggregateTransactionWrapper wrapper;
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			pTransaction->Size = entitySize;
			pTransaction->PayloadSize = payloadSize;
			test::FillWithRandomData(pTransaction->SignerPublicKey);
			test::FillWithRandomData(pTransaction->Signature);
			test::FillWithRandomData(pTransaction->TransactionsHash);

			auto* pSubTransactionBytes = reinterpret_cast<uint8_t*>(pTransaction->TransactionsPtr());
			for (uint8_t i = 0; i < numTransactions; ++i) {
				auto* pSubTransaction = reinterpret_cast<mocks::EmbeddedMockTransaction*>(pSubTransactionBytes);

				pSubTransaction->Size = transactionSize;
				pSubTransaction->Data.Size = 0;
				pSubTransaction->Version = static_cast<uint8_t>((i + 1) * 2);
				pSubTransaction->Network = static_cast<NetworkIdentifier>(100 + i);
				pSubTransaction->Type = mocks::EmbeddedMockTransaction::Entity_Type;
				test::FillWithRandomData(pSubTransaction->SignerPublicKey);
				test::FillWithRandomData(pSubTransaction->RecipientPublicKey);

				wrapper.SubTransactions.push_back(pSubTransaction);
				wrapper.SubTransactionSigners.push_back(pSubTransaction->SignerPublicKey);
				wrapper.SubTransactionRecipients.push_back(pSubTransaction->RecipientPublicKey);

				pSubTransactionBytes += transactionSize + txPaddingSize;
			}

			auto* pCosignature = pTransaction->CosignaturesPtr();
			for (auto i = 0u; i < numCosignatures; ++i) {
				*pCosignature = test::CreateRandomDetachedCosignature();
				wrapper.Cosignatures.push_back(*pCosignature);
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

	namespace {
		bool IsSizeValidDispatcher(const TransactionPlugin& plugin, AggregateTransaction& transaction, uint32_t size) {
			transaction.Size = size;
			return plugin.isSizeValid(transaction);
		}
	}

	TEST(TEST_CLASS, IsSizeValidReturnsCorrectValuesWhenTransactionIsComplete) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
		auto wrapper = CreateAggregateTransaction(3, 4);

		uint32_t embeddedTransactionSize = sizeof(mocks::EmbeddedMockTransaction);
		uint32_t expectedRealSize = sizeof(AggregateTransaction);
		expectedRealSize += 3 * (embeddedTransactionSize + utils::GetPaddingSize(embeddedTransactionSize, 8));
		expectedRealSize += 4 * SizeOf32<Cosignature>();

		// Assert:
		EXPECT_FALSE(IsSizeValidDispatcher(*pPlugin, *wrapper.pTransaction, expectedRealSize - 1));
		EXPECT_TRUE(IsSizeValidDispatcher(*pPlugin, *wrapper.pTransaction, expectedRealSize));
		EXPECT_FALSE(IsSizeValidDispatcher(*pPlugin, *wrapper.pTransaction, expectedRealSize + 1));
	}

	TEST(TEST_CLASS, IsSizeValidReturnsCorrectValuesWhenTransactionIsIncomplete) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);

		// - use vector as stand in for SizePrefixedEntity to avoid `downcast of address` ubsan error
		//   (notice that this test provides a stricter safety guarantee than necessary because real usage checks size against
		//   sizeof(EmbeddedTransaction) before calling isSizeValid)
		auto entity = test::GenerateRandomVector(sizeof(SizePrefixedEntity));

		// Assert:
		EXPECT_FALSE(IsSizeValidDispatcher(*pPlugin, reinterpret_cast<AggregateTransaction&>(entity[0]), sizeof(SizePrefixedEntity)));
	}

	TEST(TEST_CLASS, SizeIsInvalidWhenAnySubTransactionIsNotSupported) {
		// Arrange:
		TransactionRegistry registry;
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
		auto wrapper = CreateAggregateTransaction(3, 4);

		// Act + Assert:
		EXPECT_FALSE(pPlugin->isSizeValid(*wrapper.pTransaction));
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
			AggregateCosignaturesNotification::Notification_Type,
			AggregateEmbeddedTransactionsNotification::Notification_Type
		}, registry);
	}

	TEST(TEST_CLASS, CanPublishAllNotificationsWhenNeitherSubTransactionsNorCosignaturesArePresent) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto wrapper = CreateAggregateTransaction(0, 0);

		const auto& transaction = *wrapper.pTransaction;
		test::TransactionPluginTestUtils<AggregateTransactionTraits>::PublishTestBuilder builder;
		builder.addExpectation<AggregateCosignaturesNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.SignerPublicKey, notification.SignerPublicKey);
			EXPECT_EQ(0u, notification.TransactionsCount);
			EXPECT_FALSE(!!notification.TransactionsPtr);
			EXPECT_EQ(0u, notification.CosignaturesCount);
			EXPECT_FALSE(!!notification.CosignaturesPtr);
		});
		builder.addExpectation<AggregateEmbeddedTransactionsNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.TransactionsHash, notification.TransactionsHash);
			EXPECT_EQ(0u, notification.TransactionsCount);
			EXPECT_FALSE(!!notification.TransactionsPtr);
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
					EXPECT_EQ(wrapper.SubTransactions[i]->SignerPublicKey, notification.PublicKey);
				});
				builder.addExpectation<EntityNotification>(i, [i](const auto& notification) {
					// min/max version comes from MockTransactionPlugin created in CreateDefaultTransactionRegistry
					EXPECT_EQ(static_cast<NetworkIdentifier>(100 + i), notification.NetworkIdentifier);
					EXPECT_EQ((i + 1) * 2, notification.EntityVersion);
					EXPECT_EQ(0x02u, notification.MinVersion);
					EXPECT_EQ(0xFEu, notification.MaxVersion);
				});
				builder.addExpectation<AggregateEmbeddedTransactionNotification>(i, [&wrapper, i](const auto& notification) {
					EXPECT_EQ(wrapper.pTransaction->SignerPublicKey, notification.SignerPublicKey);
					EXPECT_EQ(*wrapper.SubTransactions[i], notification.Transaction);
					EXPECT_EQ(wrapper.pTransaction->CosignaturesCount(), notification.CosignaturesCount);
					EXPECT_EQ(wrapper.pTransaction->CosignaturesPtr(), notification.CosignaturesPtr);
				});
				builder.addExpectation<AccountPublicKeyNotification>(i * 2 + 1, [&wrapper, i](const auto& notification) {
					EXPECT_EQ(wrapper.SubTransactions[i]->RecipientPublicKey, notification.PublicKey);
				});
				builder.addExpectation<mocks::MockAddressNotification>(i, [&wrapper, i](const auto& notification) {
					const auto& signerPublicKey = wrapper.SubTransactions[i]->SignerPublicKey;
					auto signerAddress = PublicKeyToAddress(signerPublicKey, static_cast<NetworkIdentifier>(100 + i));
					EXPECT_EQ(signerAddress, notification.Address);
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
			AggregateEmbeddedTransactionsNotification::Notification_Type,

			// source change notification must be the first raised sub-transaction notification
			SourceChangeNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			EntityNotification::Notification_Type,
			AggregateEmbeddedTransactionNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			mocks::MockAddressNotification::Notification_Type,

			SourceChangeNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			EntityNotification::Notification_Type,
			AggregateEmbeddedTransactionNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			mocks::MockAddressNotification::Notification_Type
		}, registry);
	}

	TEST(TEST_CLASS, CanPublishAllNotificationsWhenOnlySubTransactionsArePresent) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto wrapper = CreateAggregateTransaction(2, 0);

		const auto& transaction = *wrapper.pTransaction;
		test::TransactionPluginTestUtils<AggregateTransactionTraits>::PublishTestBuilder builder;
		builder.addExpectation<AggregateCosignaturesNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.SignerPublicKey, notification.SignerPublicKey);
			EXPECT_EQ(2u, notification.TransactionsCount);
			EXPECT_EQ(transaction.TransactionsPtr(), notification.TransactionsPtr);
			EXPECT_EQ(0u, notification.CosignaturesCount);
			EXPECT_FALSE(!!notification.CosignaturesPtr);
		});
		builder.addExpectation<AggregateEmbeddedTransactionsNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.TransactionsHash, notification.TransactionsHash);
			EXPECT_EQ(2u, notification.TransactionsCount);
			EXPECT_EQ(transaction.TransactionsPtr(), notification.TransactionsPtr);
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
				// notifications should refer to cosignatories
				const auto& cosignature = wrapper.Cosignatures[i];
				builder.addExpectation<InternalPaddingNotification>(i, [&cosignature](const auto& notification) {
					EXPECT_EQ(cosignature.Version, notification.Padding);
				});
				builder.addExpectation<SignatureNotification>(i, [&cosignature, &aggregateDataHash](const auto& notification) {
					EXPECT_EQ(cosignature.SignerPublicKey, notification.SignerPublicKey);
					EXPECT_EQ(cosignature.Signature, notification.Signature);

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
			AggregateEmbeddedTransactionsNotification::Notification_Type,

			// cosignature-derived notifications are raised last (and with wrong source) for performance reasons
			InternalPaddingNotification::Notification_Type,
			SignatureNotification::Notification_Type,
			InternalPaddingNotification::Notification_Type,
			SignatureNotification::Notification_Type,
			InternalPaddingNotification::Notification_Type,
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
			EXPECT_EQ(transaction.SignerPublicKey, notification.SignerPublicKey);
			EXPECT_EQ(0u, notification.TransactionsCount);
			EXPECT_FALSE(!!notification.TransactionsPtr);
			EXPECT_EQ(3u, notification.CosignaturesCount);
			EXPECT_EQ(transaction.CosignaturesPtr(), notification.CosignaturesPtr);
		});
		builder.addExpectation<AggregateEmbeddedTransactionsNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.TransactionsHash, notification.TransactionsHash);
			EXPECT_EQ(0u, notification.TransactionsCount);
			EXPECT_FALSE(!!notification.TransactionsPtr);
		});

		AddCosignatureExpectations(builder, wrapper, aggregateDataHash, 3);

		// Act + Assert:
		builder.runTestWithHash(transaction, aggregateDataHash, registry);
	}

	// endregion

	// region publish - sub transactions and cosignatures

	TEST(TEST_CLASS, CanPublishAllNotificationsInCorrectOrderWhenSubTransactionsAndCosignaturesArePresent) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto wrapper = CreateAggregateTransaction(2, 3);

		// Act + Assert:
		test::TransactionPluginTestUtils<AggregateTransactionTraits>::AssertNotificationTypes(*wrapper.pTransaction, {
			// aggregate cosignatures notification must be the first raised notification
			AggregateCosignaturesNotification::Notification_Type,
			AggregateEmbeddedTransactionsNotification::Notification_Type,

			// source change notification must be the first raised sub-transaction notification
			SourceChangeNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			EntityNotification::Notification_Type,
			AggregateEmbeddedTransactionNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			mocks::MockAddressNotification::Notification_Type,

			SourceChangeNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			EntityNotification::Notification_Type,
			AggregateEmbeddedTransactionNotification::Notification_Type,
			AccountPublicKeyNotification::Notification_Type,
			mocks::MockAddressNotification::Notification_Type,

			// signature notifications are raised last (and with wrong source) for performance reasons
			InternalPaddingNotification::Notification_Type,
			SignatureNotification::Notification_Type,
			InternalPaddingNotification::Notification_Type,
			SignatureNotification::Notification_Type,
			InternalPaddingNotification::Notification_Type,
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
			EXPECT_EQ(transaction.SignerPublicKey, notification.SignerPublicKey);
			EXPECT_EQ(2u, notification.TransactionsCount);
			EXPECT_EQ(transaction.TransactionsPtr(), notification.TransactionsPtr);
			EXPECT_EQ(3u, notification.CosignaturesCount);
			EXPECT_EQ(transaction.CosignaturesPtr(), notification.CosignaturesPtr);
		});
		builder.addExpectation<AggregateEmbeddedTransactionsNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.TransactionsHash, notification.TransactionsHash);
			EXPECT_EQ(2u, notification.TransactionsCount);
			EXPECT_EQ(transaction.TransactionsPtr(), notification.TransactionsPtr);
		});

		AddSubTransactionExpectations(builder, wrapper, 2);
		AddCosignatureExpectations(builder, wrapper, aggregateDataHash, 3);

		// Act + Assert:
		builder.runTestWithHash(transaction, aggregateDataHash, registry);
	}

	// endregion

	// region embeddedCount

	TEST(TEST_CLASS, CanCalculateEmbeddedCountFromEmptyAggregate) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
		auto wrapper = CreateAggregateTransaction(0, 0);

		// Act:
		auto count = pPlugin->embeddedCount(*wrapper.pTransaction);

		// Assert:
		EXPECT_EQ(0u, count);
	}

	TEST(TEST_CLASS, CanCalculateEmbeddedCountFromAggregate) {
		// Arrange:
		auto registry = mocks::CreateDefaultTransactionRegistry();
		auto pPlugin = CreateAggregateTransactionPlugin(registry, Entity_Type);
		auto wrapper = CreateAggregateTransaction(2, 3);

		// Act:
		auto count = pPlugin->embeddedCount(*wrapper.pTransaction);

		// Assert:
		EXPECT_EQ(2u, count);
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
			auto aggregateDataSize = sizeof(AggregateTransaction) - VerifiableEntity::Header_Size - AggregateTransaction::Footer_Size;

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
				EXPECT_EQ(pCosignature->SignerPublicKey.data(), buffers[i].pData) << message;
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
