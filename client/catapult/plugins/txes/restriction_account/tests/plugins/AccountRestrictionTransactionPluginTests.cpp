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
			using ModifyAccountRestrictionNotification = ModifyAccountAddressRestrictionNotification;
		};

		using AddressRegularTraits = AddressTraits<AccountAddressRestrictionTransaction, AccountAddressRestrictionRegularTraits>;
		using AddressEmbeddedTraits = AddressTraits<EmbeddedAccountAddressRestrictionTransaction, AccountAddressRestrictionEmbeddedTraits>;

		template<typename TTransaction, typename TTransactionTraits>
		struct MosaicTraits : public TTransactionTraits {
			using TransactionType = TTransaction;
			using UnresolvedValueType = UnresolvedMosaicId;
			using ValueType = MosaicId;
			using ModifyAccountRestrictionValueNotification = ModifyAccountMosaicRestrictionValueNotification;
			using ModifyAccountRestrictionNotification = ModifyAccountMosaicRestrictionNotification;
		};

		using MosaicRegularTraits = MosaicTraits<AccountMosaicRestrictionTransaction, AccountMosaicRestrictionRegularTraits>;
		using MosaicEmbeddedTraits = MosaicTraits<EmbeddedAccountMosaicRestrictionTransaction, AccountMosaicRestrictionEmbeddedTraits>;

		template<typename TTransaction, typename TTransactionTraits>
		struct OperationTraits : public TTransactionTraits {
			using TransactionType = TTransaction;
			using UnresolvedValueType = EntityType;
			using ValueType = EntityType;
			using ModifyAccountRestrictionValueNotification = ModifyAccountOperationRestrictionValueNotification;
			using ModifyAccountRestrictionNotification = ModifyAccountOperationRestrictionNotification;
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

		static constexpr auto Modification_Size = sizeof(decltype(*typename TTraits::TransactionType().ModificationsPtr()));

		static auto CreateTransactionWithModifications(uint8_t numModifications) {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + numModifications * Modification_Size;
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });

			pTransaction->Size = entitySize;
			pTransaction->ModificationsCount = numModifications;
			return pTransaction;
		}

		static void AddCommonExpectations(
				typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
				const typename TTraits::TransactionType& transaction) {
			builder.template addExpectation<AccountRestrictionTypeNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.RestrictionType, notification.RestrictionType);
			});
			builder.template addExpectation<typename TTraits::ModifyAccountRestrictionNotification>([&transaction](
					const auto& notification) {
				EXPECT_EQ(transaction.SignerPublicKey, notification.Key);
				EXPECT_EQ(transaction.RestrictionType, notification.AccountRestrictionDescriptor.raw());
				EXPECT_EQ(transaction.ModificationsCount, notification.ModificationsCount);
				EXPECT_EQ(transaction.ModificationsPtr(), notification.ModificationsPtr);
			});
		}

		// endregion

	public:
		// region publish - no modifications

		static void AssertCanPublishAllNotificationsInCorrectOrderWhenNoModifications() {
			// Arrange:
			auto pTransaction = CreateTransactionWithModifications(0);

			// Act + Assert:
			test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
				AccountRestrictionTypeNotification::Notification_Type,
				TTraits::ModifyAccountRestrictionNotification::Notification_Type
			});
		}

		static void AssertCanPublishAllNotificationsWhenNoModifications() {
			// Arrange:
			auto pTransaction = CreateTransactionWithModifications(0);

			typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
			AddCommonExpectations(builder, *pTransaction);

			// Act + Assert:
			builder.runTest(*pTransaction);
		}

		// endregion

		// region publish - modifications

		static void AssertCanPublishAllNotificationsInCorrectOrderWhenModifications() {
			// Arrange:
			auto pTransaction = CreateTransactionWithModifications(4);
			for (auto i = 0u; i < 4; ++i) {
				auto action = 0 == i % 2 ? AccountRestrictionModificationAction::Add : AccountRestrictionModificationAction::Del;
				pTransaction->ModificationsPtr()[i].ModificationAction = action;
			}

			// Act + Assert:
			test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(*pTransaction, {
				AccountRestrictionTypeNotification::Notification_Type,
				TTraits::ModifyAccountRestrictionNotification::Notification_Type,
				TTraits::ModifyAccountRestrictionValueNotification::Notification_Type,
				TTraits::ModifyAccountRestrictionValueNotification::Notification_Type,
				TTraits::ModifyAccountRestrictionValueNotification::Notification_Type,
				TTraits::ModifyAccountRestrictionValueNotification::Notification_Type
			});
		}

		static void AssertCanPublishAllNotificationsWhenModifications() {
			// Arrange:
			auto pTransaction = CreateTransactionWithModifications(4);
			for (auto i = 0u; i < 4; ++i) {
				auto action = 0 == i % 2 ? AccountRestrictionModificationAction::Add : AccountRestrictionModificationAction::Del;
				pTransaction->ModificationsPtr()[i].ModificationAction = action;
			}

			const auto& transaction = *pTransaction;
			typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
			AddCommonExpectations(builder, transaction);
			for (auto i = 0u; i < 4; ++i) {
				builder.template addExpectation<typename TTraits::ModifyAccountRestrictionValueNotification>(i, [&transaction, i](
						const auto& notification) {
					EXPECT_EQ(transaction.SignerPublicKey, notification.Key);
					EXPECT_EQ(transaction.RestrictionType, notification.AccountRestrictionDescriptor.raw());
					EXPECT_EQ(transaction.ModificationsPtr()[i].ModificationAction, notification.Modification.ModificationAction);
					EXPECT_EQ(transaction.ModificationsPtr()[i].Value, notification.Modification.Value);
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
