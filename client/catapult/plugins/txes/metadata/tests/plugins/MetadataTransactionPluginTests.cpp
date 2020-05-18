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

#include "src/plugins/MetadataTransactionPlugin.h"
#include "src/model/AccountMetadataTransaction.h"
#include "src/model/MetadataNotifications.h"
#include "src/model/MosaicMetadataTransaction.h"
#include "src/model/NamespaceMetadataTransaction.h"
#include "plugins/txes/namespace/src/model/NamespaceNotifications.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS MetadataTransactionPluginTests

	// region test traits

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(AccountMetadata, 1, 1, AccountMetadata)
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(MosaicMetadata, 1, 1, MosaicMetadata)
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(NamespaceMetadata, 1, 1, NamespaceMetadata)

		template<typename TTransaction, typename TTransactionTraits>
		struct AccountTraits : public TTransactionTraits {
		private:
			using Self = AccountTraits<TTransaction, TTransactionTraits>;
			using PublishTestBuilder = typename test::TransactionPluginTestUtils<Self>::PublishTestBuilder;

		public:
			static constexpr auto Metadata_Type = MetadataType::Account;

		public:
			static uint64_t GetTargetId(const TTransaction&) {
				return 0;
			}

			static const auto& AppendExpectedCustomNotificationTypes(std::vector<NotificationType>&& notificationTypes) {
				notificationTypes.push_back(AccountPublicKeyNotification::Notification_Type);
				return notificationTypes;
			}

			static void AddCustomExpectations(PublishTestBuilder& builder, const TTransaction& transaction) {
				builder.template addExpectation<AccountPublicKeyNotification>([&transaction](const auto& notification) {
					EXPECT_EQ(transaction.TargetPublicKey, notification.PublicKey);
				});
			}
		};

		using AccountRegularTraits = AccountTraits<AccountMetadataTransaction, AccountMetadataRegularTraits>;
		using AccountEmbeddedTraits = AccountTraits<EmbeddedAccountMetadataTransaction, AccountMetadataEmbeddedTraits>;

		template<typename TTransaction, typename TTransactionTraits>
		struct MosaicTraits : public TTransactionTraits {
		private:
			using Self = MosaicTraits<TTransaction, TTransactionTraits>;
			using PublishTestBuilder = typename test::TransactionPluginTestUtils<Self>::PublishTestBuilder;

		public:
			static constexpr auto Metadata_Type = MetadataType::Mosaic;

		public:
			static uint64_t GetTargetId(const TTransaction& transaction) {
				return transaction.TargetMosaicId.unwrap();
			}

			static const auto& AppendExpectedCustomNotificationTypes(std::vector<NotificationType>&& notificationTypes) {
				notificationTypes.push_back(MosaicRequiredNotification::Notification_Type);
				return notificationTypes;
			}

			static void AddCustomExpectations(PublishTestBuilder& builder, const TTransaction& transaction) {
				builder.template addExpectation<MosaicRequiredNotification>([&transaction](const auto& notification) {
					EXPECT_EQ(transaction.TargetPublicKey, notification.Owner);
					EXPECT_EQ(MosaicId(), notification.MosaicId);
					EXPECT_EQ(transaction.TargetMosaicId, notification.UnresolvedMosaicId);
					EXPECT_EQ(0u, notification.PropertyFlagMask);
					EXPECT_EQ(MosaicRequiredNotification::MosaicType::Unresolved, notification.ProvidedMosaicType);
				});
			}
		};

		using MosaicRegularTraits = MosaicTraits<MosaicMetadataTransaction, MosaicMetadataRegularTraits>;
		using MosaicEmbeddedTraits = MosaicTraits<EmbeddedMosaicMetadataTransaction, MosaicMetadataEmbeddedTraits>;

		template<typename TTransaction, typename TTransactionTraits>
		struct NamespaceTraits : public TTransactionTraits {
		private:
			using Self = NamespaceTraits<TTransaction, TTransactionTraits>;
			using PublishTestBuilder = typename test::TransactionPluginTestUtils<Self>::PublishTestBuilder;

		public:
			static constexpr auto Metadata_Type = MetadataType::Namespace;

		public:
			static uint64_t GetTargetId(const TTransaction& transaction) {
				return transaction.TargetNamespaceId.unwrap();
			}

			static const auto& AppendExpectedCustomNotificationTypes(std::vector<NotificationType>&& notificationTypes) {
				notificationTypes.push_back(NamespaceRequiredNotification::Notification_Type);
				return notificationTypes;
			}

			static void AddCustomExpectations(PublishTestBuilder& builder, const TTransaction& transaction) {
				builder.template addExpectation<NamespaceRequiredNotification>([&transaction](const auto& notification) {
					EXPECT_EQ(transaction.TargetPublicKey, notification.Owner);
					EXPECT_EQ(transaction.TargetNamespaceId, notification.NamespaceId);
				});
			}
		};

		using NamespaceRegularTraits = NamespaceTraits<NamespaceMetadataTransaction, NamespaceMetadataRegularTraits>;
		using NamespaceEmbeddedTraits = NamespaceTraits<EmbeddedNamespaceMetadataTransaction, NamespaceMetadataEmbeddedTraits>;
	}

	// endregion

	// region test macro expansion

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ONLY_EMBEDDABLE(TEST_CLASS, Account, _Account, Entity_Type_Account_Metadata)
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ONLY_EMBEDDABLE(TEST_CLASS, Mosaic, _Mosaic, Entity_Type_Mosaic_Metadata)
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ONLY_EMBEDDABLE(TEST_CLASS, Namespace, _Namespace, Entity_Type_Namespace_Metadata)

	// endregion

	// region publish - account

#define METADATA_PLUGIN_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular_Account) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountRegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_Account) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountEmbeddedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Regular_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicRegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicEmbeddedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Regular_Namespace) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NamespaceRegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_Namespace) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NamespaceEmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		template<typename TTraits>
		auto CreateTransactionWithValue(uint16_t valueSize) {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + valueSize;
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });

			pTransaction->Size = entitySize;
			pTransaction->ValueSize = valueSize;
			return pTransaction;
		}
	}

	METADATA_PLUGIN_BASED_TEST(CanPublishAllNotificationsInCorrectOrder) {
		// Arrange:
		auto pTransaction = CreateTransactionWithValue<TTraits>(11);

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, TTraits::AppendExpectedCustomNotificationTypes({
			MetadataSizesNotification::Notification_Type,
			MetadataValueNotification::Notification_Type
		}));
	}

	METADATA_PLUGIN_BASED_TEST(CanPublishAllNotifications) {
		// Arrange:
		auto pTransaction = CreateTransactionWithValue<TTraits>(11);

		const auto& transaction = *pTransaction;
		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		TTraits::AddCustomExpectations(builder, transaction);
		builder.template addExpectation<MetadataSizesNotification>([&transaction](const auto& notification) {
			EXPECT_EQ(transaction.ValueSizeDelta, notification.ValueSizeDelta);
			EXPECT_EQ(11u, notification.ValueSize);
		});
		builder.template addExpectation<MetadataValueNotification>([&transaction](const auto& notification) {
			// partial metadata key
			EXPECT_EQ(transaction.SignerPublicKey, notification.PartialMetadataKey.SourcePublicKey);
			EXPECT_EQ(transaction.TargetPublicKey, notification.PartialMetadataKey.TargetPublicKey);
			EXPECT_EQ(transaction.ScopedMetadataKey, notification.PartialMetadataKey.ScopedMetadataKey);

			// metadata target
			EXPECT_EQ(TTraits::GetTargetId(transaction), notification.MetadataTarget.Id);
			EXPECT_EQ(TTraits::Metadata_Type, notification.MetadataTarget.Type);

			// value
			EXPECT_EQ(transaction.ValueSizeDelta, notification.ValueSizeDelta);
			EXPECT_EQ(11u, notification.ValueSize);
			EXPECT_TRUE(!!notification.ValuePtr);
			EXPECT_EQ(transaction.ValuePtr(), notification.ValuePtr);
		});

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region additionalRequiredCosignatories

#define METADATA_EMBEDDED_PLUGIN_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Account) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountEmbeddedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicEmbeddedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Namespace) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NamespaceEmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	METADATA_EMBEDDED_PLUGIN_BASED_TEST(CanExtractAdditionalRequiredCosignatoriesFromEmbedded) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		// Act:
		auto additionalCosignatories = pPlugin->additionalRequiredCosignatories(transaction);

		// Assert:
		EXPECT_EQ(utils::KeySet{ transaction.TargetPublicKey }, additionalCosignatories);
	}

	// endregion
}}
