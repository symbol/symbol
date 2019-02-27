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

#include "src/plugins/PropertyTransactionPlugin.h"
#include "src/model/AddressPropertyTransaction.h"
#include "src/model/MosaicPropertyTransaction.h"
#include "src/model/PropertyNotifications.h"
#include "src/model/TransactionTypePropertyTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

#define TEST_CLASS PropertyTransactionPluginTests

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS_WITH_PREFIXED_TRAITS(AddressProperty, 1, 1, AddressProperty)
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS_WITH_PREFIXED_TRAITS(MosaicProperty, 1, 1, MosaicProperty)
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS_WITH_PREFIXED_TRAITS(TransactionTypeProperty, 1, 1, TransactionTypeProperty)

		template<typename TTransaction, typename TTransactionTraits>
		struct AddressTraits : public TTransactionTraits {
			using TransactionType = TTransaction;
			using UnresolvedValueType = UnresolvedAddress;
			using ValueType = Address;
			using ModifyPropertyValueNotification = model::ModifyAddressPropertyValueNotification;
			using ModifyPropertyNotification = model::ModifyAddressPropertyNotification;
		};

		using AddressRegularTraits = AddressTraits<model::AddressPropertyTransaction, AddressPropertyRegularTraits>;
		using AddressEmbeddedTraits = AddressTraits<model::EmbeddedAddressPropertyTransaction, AddressPropertyEmbeddedTraits>;

		template<typename TTransaction, typename TTransactionTraits>
		struct MosaicTraits : public TTransactionTraits {
			using TransactionType = TTransaction;
			using UnresolvedValueType = UnresolvedMosaicId;
			using ValueType = MosaicId;
			using ModifyPropertyValueNotification = model::ModifyMosaicPropertyValueNotification;
			using ModifyPropertyNotification = model::ModifyMosaicPropertyNotification;
		};

		using MosaicRegularTraits = MosaicTraits<model::MosaicPropertyTransaction, MosaicPropertyRegularTraits>;
		using MosaicEmbeddedTraits = MosaicTraits<model::EmbeddedMosaicPropertyTransaction, MosaicPropertyEmbeddedTraits>;

		template<typename TTransaction, typename TTransactionTraits>
		struct TransactionTypeTraits : public TTransactionTraits {
			using TransactionType = TTransaction;
			using UnresolvedValueType = model::EntityType;
			using ValueType = model::EntityType;
			using ModifyPropertyValueNotification = model::ModifyTransactionTypePropertyValueNotification;
			using ModifyPropertyNotification = model::ModifyTransactionTypePropertyNotification;
		};

		using TransactionTypeRegularTraits = TransactionTypeTraits<
			model::TransactionTypePropertyTransaction,
			TransactionTypePropertyRegularTraits>;
		using TransactionTypeEmbeddedTraits = TransactionTypeTraits<
			model::EmbeddedTransactionTypePropertyTransaction,
			TransactionTypePropertyEmbeddedTraits>;
	}

	template<typename TTraits>
	class PropertyTransactionPluginTests {
	private:
		static uint32_t PropertyModificationSize() {
			return sizeof(decltype(*typename TTraits::TransactionType().ModificationsPtr()));
		}

		static auto CreatePropertyTransaction() {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + 12 * PropertyModificationSize();
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			pTransaction->Size = entitySize;
			pTransaction->ModificationsCount = 12;
			auto* pModification = pTransaction->ModificationsPtr();
			for (auto i = 0u; i < 12; ++i) {
				pModification->ModificationType = model::PropertyModificationType(i);
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
			EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 123 * PropertyModificationSize(), realSize);
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

		// region property type notification

		static void AssertCanPublishPropertyTypeNotification() {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<model::PropertyTypeNotification> sub;
			auto pPlugin = TTraits::CreatePlugin();

			auto pTransaction = CreatePropertyTransaction();

			// Act:
			test::PublishTransaction(*pPlugin, *pTransaction, sub);

			// Assert:
			ASSERT_EQ(2u + 12, sub.numNotifications());
			ASSERT_EQ(1u, sub.numMatchingNotifications());
			EXPECT_EQ(pTransaction->PropertyType, sub.matchingNotifications()[0].PropertyType);
		}

		// endregion

		// region typed property modifications notification

		static void AssertCanPublishTypedPropertyModificationsNotification() {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<typename TTraits::ModifyPropertyNotification> sub;
			auto pPlugin = TTraits::CreatePlugin();

			auto pTransaction = CreatePropertyTransaction();

			// Act:
			test::PublishTransaction(*pPlugin, *pTransaction, sub);

			// Assert:
			ASSERT_EQ(2u + 12, sub.numNotifications());
			ASSERT_EQ(1u, sub.numMatchingNotifications());

			const auto& notification = sub.matchingNotifications()[0];
			EXPECT_EQ(pTransaction->Signer, notification.Key);
			EXPECT_EQ(pTransaction->PropertyType, notification.PropertyDescriptor.raw());
			EXPECT_EQ(pTransaction->ModificationsCount, notification.ModificationsCount);
			EXPECT_EQ(pTransaction->ModificationsPtr(), notification.ModificationsPtr);
		}

		// endregion

		// region typed property value modification notification

		static void AssertCanPublishTypedPropertyValueModificationNotification() {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<typename TTraits::ModifyPropertyValueNotification> sub;
			auto pPlugin = TTraits::CreatePlugin();

			auto pTransaction = CreatePropertyTransaction();

			// Act:
			test::PublishTransaction(*pPlugin, *pTransaction, sub);

			// Assert:
			ASSERT_EQ(2u + 12, sub.numNotifications());
			ASSERT_EQ(12u, sub.numMatchingNotifications());

			for (auto i = 0u; i < 12; ++i) {
				const auto& notification = sub.matchingNotifications()[i];
				EXPECT_EQ(pTransaction->Signer, notification.Key);
				EXPECT_EQ(pTransaction->PropertyType, notification.PropertyDescriptor.raw());
				EXPECT_EQ(model::PropertyModificationType(i), notification.Modification.ModificationType);
			}
		}

		// endregion
	};

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_WITH_PREFIXED_TRAITS(
			TEST_CLASS,
			Address,
			_Address,
			model::Entity_Type_Address_Property)
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_WITH_PREFIXED_TRAITS(TEST_CLASS, Mosaic, _Mosaic, model::Entity_Type_Mosaic_Property)
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_WITH_PREFIXED_TRAITS(
			TEST_CLASS,
			TransactionType,
			_TransactionType,
			model::Entity_Type_Transaction_Type_Property)

#define MAKE_PROPERTY_TRANSACTION_PLUGIN_TEST(TRAITS_PREFIX, TEST_POSTFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##_Regular##TEST_POSTFIX) { \
		PropertyTransactionPluginTests<TRAITS_PREFIX##RegularTraits>::Assert##TEST_NAME(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Embedded##TEST_POSTFIX) { \
		PropertyTransactionPluginTests<TRAITS_PREFIX##EmbeddedTraits>::Assert##TEST_NAME(); \
	}

#define DEFINE_PROPERTY_TRANSACTION_PLUGIN_TESTS(TRAITS_PREFIX, TEST_POSTFIX) \
	MAKE_PROPERTY_TRANSACTION_PLUGIN_TEST(TRAITS_PREFIX, TEST_POSTFIX, CanCalculateSize) \
	MAKE_PROPERTY_TRANSACTION_PLUGIN_TEST(TRAITS_PREFIX, TEST_POSTFIX, CanExtractAccounts) \
	MAKE_PROPERTY_TRANSACTION_PLUGIN_TEST(TRAITS_PREFIX, TEST_POSTFIX, CanPublishPropertyTypeNotification) \
	MAKE_PROPERTY_TRANSACTION_PLUGIN_TEST(TRAITS_PREFIX, TEST_POSTFIX, CanPublishTypedPropertyModificationsNotification) \
	MAKE_PROPERTY_TRANSACTION_PLUGIN_TEST(TRAITS_PREFIX, TEST_POSTFIX, CanPublishTypedPropertyValueModificationNotification)

	DEFINE_PROPERTY_TRANSACTION_PLUGIN_TESTS(Address, _Address)
	DEFINE_PROPERTY_TRANSACTION_PLUGIN_TESTS(Mosaic, _Mosaic)
	DEFINE_PROPERTY_TRANSACTION_PLUGIN_TESTS(TransactionType, _TransactionType)
}}
