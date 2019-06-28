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

namespace catapult { namespace plugins {

#define TEST_CLASS AccountRestrictionTransactionPluginTests

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(AccountAddressRestriction, 1, 1, AccountAddressRestriction)
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(AccountMosaicRestriction, 1, 1, AccountMosaicRestriction)
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(AccountOperationRestriction, 1, 1, AccountOperationRestriction)

		template<typename TTransaction, typename TTransactionTraits>
		struct AddressTraits : public TTransactionTraits {
			using TransactionType = TTransaction;
			using UnresolvedValueType = UnresolvedAddress;
			using ValueType = Address;
			using ModifyAccountRestrictionValueNotification = model::ModifyAccountAddressRestrictionValueNotification;
			using ModifyAccountRestrictionNotification = model::ModifyAccountAddressRestrictionNotification;
		};

		using AddressRegularTraits = AddressTraits<model::AccountAddressRestrictionTransaction, AccountAddressRestrictionRegularTraits>;
		using AddressEmbeddedTraits = AddressTraits<
			model::EmbeddedAccountAddressRestrictionTransaction,
			AccountAddressRestrictionEmbeddedTraits>;

		template<typename TTransaction, typename TTransactionTraits>
		struct MosaicTraits : public TTransactionTraits {
			using TransactionType = TTransaction;
			using UnresolvedValueType = UnresolvedMosaicId;
			using ValueType = MosaicId;
			using ModifyAccountRestrictionValueNotification = model::ModifyAccountMosaicRestrictionValueNotification;
			using ModifyAccountRestrictionNotification = model::ModifyAccountMosaicRestrictionNotification;
		};

		using MosaicRegularTraits = MosaicTraits<model::AccountMosaicRestrictionTransaction, AccountMosaicRestrictionRegularTraits>;
		using MosaicEmbeddedTraits = MosaicTraits<
			model::EmbeddedAccountMosaicRestrictionTransaction,
			AccountMosaicRestrictionEmbeddedTraits>;

		template<typename TTransaction, typename TTransactionTraits>
		struct OperationTraits : public TTransactionTraits {
			using TransactionType = TTransaction;
			using UnresolvedValueType = model::EntityType;
			using ValueType = model::EntityType;
			using ModifyAccountRestrictionValueNotification = model::ModifyAccountOperationRestrictionValueNotification;
			using ModifyAccountRestrictionNotification = model::ModifyAccountOperationRestrictionNotification;
		};

		using OperationRegularTraits = OperationTraits<
			model::AccountOperationRestrictionTransaction,
			AccountOperationRestrictionRegularTraits>;
		using OperationEmbeddedTraits = OperationTraits<
			model::EmbeddedAccountOperationRestrictionTransaction,
			AccountOperationRestrictionEmbeddedTraits>;
	}

	template<typename TTraits>
	class AccountRestrictionTransactionPluginTests {
	private:
		static uint32_t AccountRestrictionModificationSize() {
			return sizeof(decltype(*typename TTraits::TransactionType().ModificationsPtr()));
		}

		static auto CreateAccountRestrictionTransaction() {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + 12 * AccountRestrictionModificationSize();
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			pTransaction->Size = entitySize;
			pTransaction->ModificationsCount = 12;
			auto* pModification = pTransaction->ModificationsPtr();
			for (auto i = 0u; i < 12; ++i) {
				pModification->ModificationType = model::AccountRestrictionModificationType(i);
				++pModification;
			}

			return pTransaction;
		}

	public:
		// region TransactionPlugin

		static void AssertCanCalculateSize() {
			// Arrange:
			auto pPlugin = TTraits::CreatePlugin();

			typename TTraits::TransactionType transaction;
			transaction.ModificationsCount = 123;

			// Act:
			auto realSize = pPlugin->calculateRealSize(transaction);

			// Assert:
			EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 123 * AccountRestrictionModificationSize(), realSize);
		}

		// endregion

		// region accounts extraction

		static void AssertCanExtractAccounts() {
			// Arrange:
			mocks::MockNotificationSubscriber sub;
			auto pPlugin = TTraits::CreatePlugin();

			typename TTraits::TransactionType transaction;
			transaction.ModificationsCount = 0;

			// Act:
			test::PublishTransaction(*pPlugin, transaction, sub);

			// Assert:
			EXPECT_EQ(2u, sub.numNotifications());
			EXPECT_EQ(0u, sub.numAddresses());
			EXPECT_EQ(0u, sub.numKeys());
		}

		// endregion

		// region restriction type notification

		static void AssertCanPublishAccountRestrictionTypeNotification() {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<model::AccountRestrictionTypeNotification> sub;
			auto pPlugin = TTraits::CreatePlugin();

			auto pTransaction = CreateAccountRestrictionTransaction();

			// Act:
			test::PublishTransaction(*pPlugin, *pTransaction, sub);

			// Assert:
			ASSERT_EQ(2u + 12, sub.numNotifications());
			ASSERT_EQ(1u, sub.numMatchingNotifications());
			EXPECT_EQ(pTransaction->RestrictionType, sub.matchingNotifications()[0].AccountRestrictionType);
		}

		// endregion

		// region typed account restriction modifications notification

		static void AssertCanPublishTypedAccountRestrictionModificationsNotification() {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<typename TTraits::ModifyAccountRestrictionNotification> sub;
			auto pPlugin = TTraits::CreatePlugin();

			auto pTransaction = CreateAccountRestrictionTransaction();

			// Act:
			test::PublishTransaction(*pPlugin, *pTransaction, sub);

			// Assert:
			ASSERT_EQ(2u + 12, sub.numNotifications());
			ASSERT_EQ(1u, sub.numMatchingNotifications());

			const auto& notification = sub.matchingNotifications()[0];
			EXPECT_EQ(pTransaction->Signer, notification.Key);
			EXPECT_EQ(pTransaction->RestrictionType, notification.AccountRestrictionDescriptor.raw());
			EXPECT_EQ(pTransaction->ModificationsCount, notification.ModificationsCount);
			EXPECT_EQ(pTransaction->ModificationsPtr(), notification.ModificationsPtr);
		}

		// endregion

		// region typed account restriction value modification notification

		static void AssertCanPublishTypedAccountRestrictionValueModificationNotification() {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<typename TTraits::ModifyAccountRestrictionValueNotification> sub;
			auto pPlugin = TTraits::CreatePlugin();

			auto pTransaction = CreateAccountRestrictionTransaction();

			// Act:
			test::PublishTransaction(*pPlugin, *pTransaction, sub);

			// Assert:
			ASSERT_EQ(2u + 12, sub.numNotifications());
			ASSERT_EQ(12u, sub.numMatchingNotifications());

			for (auto i = 0u; i < 12; ++i) {
				const auto& notification = sub.matchingNotifications()[i];
				EXPECT_EQ(pTransaction->Signer, notification.Key);
				EXPECT_EQ(pTransaction->RestrictionType, notification.AccountRestrictionDescriptor.raw());
				EXPECT_EQ(model::AccountRestrictionModificationType(i), notification.Modification.ModificationType);
			}
		}

		// endregion
	};

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Address, _Address, model::Entity_Type_Account_Address_Restriction)
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Mosaic, _Mosaic, model::Entity_Type_Account_Mosaic_Restriction)
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Operation, _Operation, model::Entity_Type_Account_Operation_Restriction)

#define MAKE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TEST(TRAITS_PREFIX, TEST_POSTFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##_Regular##TEST_POSTFIX) { \
		AccountRestrictionTransactionPluginTests<TRAITS_PREFIX##RegularTraits>::Assert##TEST_NAME(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Embedded##TEST_POSTFIX) { \
		AccountRestrictionTransactionPluginTests<TRAITS_PREFIX##EmbeddedTraits>::Assert##TEST_NAME(); \
	}

#define DEFINE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TESTS(TRAITS_PREFIX, TEST_POSTFIX) \
	MAKE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TEST(TRAITS_PREFIX, TEST_POSTFIX, CanCalculateSize) \
	MAKE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TEST(TRAITS_PREFIX, TEST_POSTFIX, CanExtractAccounts) \
	MAKE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TEST(TRAITS_PREFIX, TEST_POSTFIX, CanPublishAccountRestrictionTypeNotification) \
	MAKE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TEST( \
			TRAITS_PREFIX, \
			TEST_POSTFIX, \
			CanPublishTypedAccountRestrictionModificationsNotification) \
	MAKE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TEST( \
			TRAITS_PREFIX, \
			TEST_POSTFIX, \
			CanPublishTypedAccountRestrictionValueModificationNotification)

	DEFINE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TESTS(Address, _Address)
	DEFINE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TESTS(Mosaic, _Mosaic)
	DEFINE_ACCOUNT_RESTRICTION_TRANSACTION_PLUGIN_TESTS(Operation, _Operation)
}}
