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

#include "src/plugins/AccountRestrictionTransactionPlugin.h"
#include "src/model/AccountAddressRestrictionTransaction.h"
#include "src/model/AccountMosaicRestrictionTransaction.h"
#include "src/model/AccountOperationRestrictionTransaction.h"
#include "src/model/AccountRestrictionNotifications.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS AccountRestrictionTransactionPluginTests

	// region test traits

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(AccountAddressRestriction, 1, 1, AccountAddressRestriction)
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(AccountMosaicRestriction, 1, 1, AccountMosaicRestriction)
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(AccountOperationRestriction, 1, 1, AccountOperationRestriction)

		template<typename TTransaction, typename TTransactionTraits>
		struct AddressTraits : public TTransactionTraits {
			using TransactionType = TTransaction;
			using UnresolvedValueType = UnresolvedAddress;
			using ValueType = Address;
			using ModifyAccountRestrictionValueNotification = ModifyAccountAddressRestrictionValueNotification;
			using ModifyAccountRestrictionsNotification = ModifyAccountAddressRestrictionsNotification;
		};

		using AddressRegularTraits = AddressTraits<AccountAddressRestrictionTransaction, AccountAddressRestrictionRegularTraits>;
		using AddressEmbeddedTraits = AddressTraits<EmbeddedAccountAddressRestrictionTransaction, AccountAddressRestrictionEmbeddedTraits>;

		template<typename TTransaction, typename TTransactionTraits>
		struct MosaicTraits : public TTransactionTraits {
			using TransactionType = TTransaction;
			using UnresolvedValueType = UnresolvedMosaicId;
			using ValueType = MosaicId;
			using ModifyAccountRestrictionValueNotification = ModifyAccountMosaicRestrictionValueNotification;
			using ModifyAccountRestrictionsNotification = ModifyAccountMosaicRestrictionsNotification;
		};

		using MosaicRegularTraits = MosaicTraits<AccountMosaicRestrictionTransaction, AccountMosaicRestrictionRegularTraits>;
		using MosaicEmbeddedTraits = MosaicTraits<EmbeddedAccountMosaicRestrictionTransaction, AccountMosaicRestrictionEmbeddedTraits>;

		template<typename TTransaction, typename TTransactionTraits>
		struct OperationTraits : public TTransactionTraits {
			using TransactionType = TTransaction;
			using UnresolvedValueType = EntityType;
			using ValueType = EntityType;
			using ModifyAccountRestrictionValueNotification = ModifyAccountOperationRestrictionValueNotification;
			using ModifyAccountRestrictionsNotification = ModifyAccountOperationRestrictionsNotification;
		};

		using OperationRegularTraits = OperationTraits<AccountOperationRestrictionTransaction, AccountOperationRestrictionRegularTraits>;
		using OperationEmbeddedTraits = OperationTraits<
			EmbeddedAccountOperationRestrictionTransaction,
			AccountOperationRestrictionEmbeddedTraits>;
	}

	// endregion

	template<typename TTraits>
	class AccountRestrictionTransactionPluginTests {
	private:
		// region test utils

		static constexpr auto Modification_Size = sizeof(decltype(*typename TTraits::TransactionType().RestrictionAdditionsPtr()));

		static auto CreateTransactionWithModifications(uint8_t numAdditions, uint8_t numDeletions) {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + (numAdditions + numDeletions) * Modification_Size;
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });

			pTransaction->Size = entitySize;
			pTransaction->RestrictionAdditionsCount = numAdditions;
			pTransaction->RestrictionDeletionsCount = numDeletions;
			return pTransaction;
		}

		static void AddCommonExpectations(
				typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
				const typename TTraits::TransactionType& transaction) {
			builder.template addExpectation<InternalPaddingNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.AccountRestrictionTransactionBody_Reserved1, notification.Padding);
			});
			builder.template addExpectation<AccountRestrictionModificationNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.RestrictionFlags, notification.RestrictionFlags);
				EXPECT_EQ(transaction.RestrictionAdditionsCount, notification.RestrictionAdditionsCount);
				EXPECT_EQ(transaction.RestrictionDeletionsCount, notification.RestrictionDeletionsCount);
			});
			builder.template addExpectation<typename TTraits::ModifyAccountRestrictionsNotification>([&transaction](
					const auto& notification) {
				EXPECT_EQ(GetSignerAddress(transaction), notification.Address);
				EXPECT_EQ(transaction.RestrictionFlags, notification.AccountRestrictionDescriptor.raw());
				EXPECT_EQ(transaction.RestrictionAdditionsCount, notification.RestrictionAdditionsCount);
				EXPECT_EQ(transaction.RestrictionAdditionsPtr(), notification.RestrictionAdditionsPtr);
				EXPECT_EQ(transaction.RestrictionDeletionsCount, notification.RestrictionDeletionsCount);
				EXPECT_EQ(transaction.RestrictionDeletionsPtr(), notification.RestrictionDeletionsPtr);
			});
		}

		// endregion

	public:
		// region publish - no modifications

		static void AssertCanPublishAllNotificationsInCorrectOrderWhenNoModifications() {
			// Arrange:
			auto pTransaction = CreateTransactionWithModifications(0, 0);

			// Act + Assert:
			test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
				InternalPaddingNotification::Notification_Type,
				AccountRestrictionModificationNotification::Notification_Type,
				TTraits::ModifyAccountRestrictionsNotification::Notification_Type
			});
		}

		static void AssertCanPublishAllNotificationsWhenNoModifications() {
			// Arrange:
			auto pTransaction = CreateTransactionWithModifications(0, 0);

			typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
			AddCommonExpectations(builder, *pTransaction);

			// Act + Assert:
			builder.runTest(*pTransaction);
		}

		// endregion

		// region publish - modifications

		static void AssertCanPublishAllNotificationsInCorrectOrderWhenModifications() {
			// Arrange:
			auto pTransaction = CreateTransactionWithModifications(3, 2);

			// Act + Assert:
			test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
				InternalPaddingNotification::Notification_Type,
				AccountRestrictionModificationNotification::Notification_Type,
				TTraits::ModifyAccountRestrictionsNotification::Notification_Type,
				TTraits::ModifyAccountRestrictionValueNotification::Notification_Type,
				TTraits::ModifyAccountRestrictionValueNotification::Notification_Type,
				TTraits::ModifyAccountRestrictionValueNotification::Notification_Type,
				TTraits::ModifyAccountRestrictionValueNotification::Notification_Type,
				TTraits::ModifyAccountRestrictionValueNotification::Notification_Type
			});
		}

		static void AssertCanPublishAllNotificationsWhenModifications() {
			// Arrange:
			auto pTransaction = CreateTransactionWithModifications(3, 2);

			const auto& transaction = *pTransaction;
			typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
			AddCommonExpectations(builder, transaction);
			for (auto i = 0u; i < 3; ++i) {
				builder.template addExpectation<typename TTraits::ModifyAccountRestrictionValueNotification>(i, [&transaction, i](
						const auto& notification) {
					EXPECT_EQ(GetSignerAddress(transaction), notification.Address);
					EXPECT_EQ(transaction.RestrictionFlags, notification.AccountRestrictionDescriptor.raw());
					EXPECT_EQ(transaction.RestrictionAdditionsPtr()[i], notification.RestrictionValue);
					EXPECT_EQ(AccountRestrictionModificationAction::Add, notification.Action);
				});
			}

			for (auto i = 0u; i < 2; ++i) {
				builder.template addExpectation<typename TTraits::ModifyAccountRestrictionValueNotification>(3 + i, [&transaction, i](
						const auto& notification) {
					EXPECT_EQ(GetSignerAddress(transaction), notification.Address);
					EXPECT_EQ(transaction.RestrictionFlags, notification.AccountRestrictionDescriptor.raw());
					EXPECT_EQ(transaction.RestrictionDeletionsPtr()[i], notification.RestrictionValue);
					EXPECT_EQ(AccountRestrictionModificationAction::Del, notification.Action);
				});
			}

			// Act + Assert:
			builder.runTest(transaction);
		}

		// endregion
	};

	// region test macro expansion

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Address, _Address, Entity_Type_Account_Address_Restriction)
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Mosaic, _Mosaic, Entity_Type_Account_Mosaic_Restriction)
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Operation, _Operation, Entity_Type_Account_Operation_Restriction)

#define MAKE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TEST(TRAITS_PREFIX, TEST_POSTFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##_Regular##TEST_POSTFIX) { \
		AccountRestrictionTransactionPluginTests<TRAITS_PREFIX##RegularTraits>::Assert##TEST_NAME(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Embedded##TEST_POSTFIX) { \
		AccountRestrictionTransactionPluginTests<TRAITS_PREFIX##EmbeddedTraits>::Assert##TEST_NAME(); \
	}

#define DEFINE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TESTS(TRAITS_PREFIX, TEST_POSTFIX) \
	MAKE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TEST( \
			TRAITS_PREFIX, \
			TEST_POSTFIX, \
			CanPublishAllNotificationsInCorrectOrderWhenNoModifications) \
	MAKE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TEST(TRAITS_PREFIX, TEST_POSTFIX, CanPublishAllNotificationsWhenNoModifications) \
	MAKE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TEST( \
			TRAITS_PREFIX, \
			TEST_POSTFIX, \
			CanPublishAllNotificationsInCorrectOrderWhenModifications) \
	MAKE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TEST(TRAITS_PREFIX, TEST_POSTFIX, CanPublishAllNotificationsWhenModifications) \

	DEFINE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TESTS(Address, _Address)
	DEFINE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TESTS(Mosaic, _Mosaic)
	DEFINE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TESTS(Operation, _Operation)

	// endregion
}}
