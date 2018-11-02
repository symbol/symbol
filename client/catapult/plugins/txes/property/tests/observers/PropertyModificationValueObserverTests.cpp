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

#include "src/observers/Observers.h"
#include "catapult/model/Address.h"
#include "tests/test/PropertyCacheTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS PropertyModificationValueObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::PropertyCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(AddressPropertyValueModification,)
	DEFINE_COMMON_OBSERVER_TESTS(MosaicPropertyValueModification,)
	DEFINE_COMMON_OBSERVER_TESTS(TransactionTypePropertyValueModification,)

	namespace {
		constexpr auto Add = model::PropertyModificationType::Add;
		constexpr auto Del = model::PropertyModificationType::Del;
		constexpr auto Num_Default_Entries = 5u;

		struct AddressPropertyTraits : public test::BaseAddressPropertyTraits {
			static constexpr auto CreateObserver = CreateAddressPropertyValueModificationObserver;

			using NotificationType = model::ModifyAddressPropertyValueNotification;
		};

		struct MosaicPropertyTraits : public test::BaseMosaicPropertyTraits {
			static constexpr auto CreateObserver = CreateMosaicPropertyValueModificationObserver;

			using NotificationType = model::ModifyMosaicPropertyValueNotification;
		};

		struct TransactionTypePropertyTraits : public test::BaseTransactionTypePropertyTraits {
			static constexpr auto CreateObserver = CreateTransactionTypePropertyValueModificationObserver;

			using NotificationType = model::ModifyTransactionTypePropertyValueNotification;
		};

		bool IsInsert(NotifyMode notifyMode, model::PropertyModificationType modificationType) {
			return
					(NotifyMode::Commit == notifyMode && Add == modificationType) ||
					(NotifyMode::Rollback == notifyMode && Del == modificationType);
		}

		template<typename TPropertyValueTraits>
		void AssertCache(
				size_t expectedSize,
				const cache::PropertyCacheDelta& delta,
				NotifyMode notifyMode,
				const Key& key,
				const model::PropertyModification<typename TPropertyValueTraits::ValueType>& modification,
				bool shouldContainCacheEntry) {
			// Assert:
			using ValueType = typename TPropertyValueTraits::ValueType;
			auto address = model::PublicKeyToAddress(key, model::NetworkIdentifier::Zero);

			auto iter = delta.find(address);
			if (!shouldContainCacheEntry) {
				EXPECT_FALSE(!!iter.tryGet());
				return;
			}

			const auto& accountProperties = iter.get();
			auto typedProperties = accountProperties.template property<ValueType>(TPropertyValueTraits::PropertyType());
			if (IsInsert(notifyMode, modification.ModificationType))
				EXPECT_TRUE(typedProperties.contains(modification.Value));
			else
				EXPECT_FALSE(typedProperties.contains(modification.Value));

			EXPECT_EQ(expectedSize, typedProperties.size());
		}

		enum class CachePolicy { Populate, Empty };

		template<typename TOperationTraits, typename TPropertyValueTraits, typename TModificationFactory>
		void RunTest(
				size_t expectedSize,
				NotifyMode notifyMode,
				size_t numInitialValues,
				bool shouldContainCacheEntry,
				TModificationFactory modificationFactory) {
			// Arrange:
			ObserverTestContext context(notifyMode);
			auto values = test::GenerateRandomDataVector<typename TPropertyValueTraits::ValueType>(numInitialValues);
			auto key = test::GenerateRandomData<Key_Size>();

			if (0 < numInitialValues)
				test::PopulateCache<TPropertyValueTraits, TOperationTraits>(context.cache(), key, values);

			auto modification = modificationFactory(values);
			auto notification = test::CreateNotification<TPropertyValueTraits, TOperationTraits>(key, modification);
			auto pObserver = TPropertyValueTraits::CreateObserver();

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			AssertCache<TPropertyValueTraits>(
					expectedSize,
					context.cache().sub<cache::PropertyCache>(),
					notifyMode,
					key,
					modification,
					shouldContainCacheEntry);
		}

		template<typename TOperationTraits, typename TPropertyValueTraits, typename TModificationFactory>
		void RunTest(size_t expectedSize, NotifyMode notifyMode, TModificationFactory modificationFactory) {
			// Act:
			RunTest<TOperationTraits, TPropertyValueTraits>(expectedSize, notifyMode, Num_Default_Entries, true, modificationFactory);
		}

		template<typename TOperationTraits>
		void AssertObserverDoesNotRemoveNonEmptyAccountProperties(NotifyMode notifyMode) {
			// Arrange:
			ObserverTestContext context(notifyMode);
			auto filteredAddress = test::GenerateRandomData<Address_Decoded_Size>();
			auto key = test::GenerateRandomData<Key_Size>();
			auto& propertyCacheDelta = context.cache().sub<cache::PropertyCache>();
			auto accountAddress = model::PublicKeyToAddress(key, model::NetworkIdentifier::Zero);
			propertyCacheDelta.insert(state::AccountProperties(accountAddress));

			auto& accountProperties = propertyCacheDelta.find(accountAddress).get();
			TOperationTraits::Add(accountProperties.property(model::PropertyType::Address), state::ToVector(filteredAddress));
			TOperationTraits::Add(accountProperties.property(model::PropertyType::MosaicId), test::GenerateRandomVector(sizeof(MosaicId)));

			auto modificationType = NotifyMode::Commit == notifyMode ? Del : Add;
			auto modification = model::PropertyModification<Address>{ modificationType, filteredAddress };
			auto completeType = TOperationTraits::CompletePropertyType(model::PropertyType::Address);
			model::ModifyAddressPropertyValueNotification notification{ key, completeType, modification };
			auto pObserver = CreateAddressPropertyValueModificationObserver();

			// Act: cache entry is not removed, address property is empty but mosaic property is not
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			auto iter = context.cache().sub<cache::PropertyCache>().find(accountAddress);
			ASSERT_TRUE(!!iter.tryGet());

			const auto& properties = iter.get();
			EXPECT_FALSE(properties.isEmpty());
			EXPECT_TRUE(properties.property(model::PropertyType::Address).values().empty());
			EXPECT_EQ(1u, properties.property(model::PropertyType::MosaicId).values().size());
		}

		template<typename TOperationTraits, typename TPropertyValueTraits>
		void AssertObserverAddsAccountProperties(NotifyMode notifyMode) {
			// Act:
			RunTest<TOperationTraits, TPropertyValueTraits>(1, notifyMode, 0, true, [notifyMode](const auto&) {
				auto modificationType = NotifyMode::Commit == notifyMode ? Add : Del;
				return model::PropertyModification<typename TPropertyValueTraits::ValueType>{
					modificationType,
					TPropertyValueTraits::RandomValue()
				};
			});
		}

		template<typename TOperationTraits, typename TPropertyValueTraits>
		void AssertObserverRemovesEmptyAccountProperties(NotifyMode notifyMode) {
			// Act:
			RunTest<TOperationTraits, TPropertyValueTraits>(0, notifyMode, 1, false, [notifyMode](const auto& values) {
				auto modificationType = NotifyMode::Commit == notifyMode ? Del : Add;
				return model::PropertyModification<typename TPropertyValueTraits::ValueType>({ modificationType, values[0] });
			});
		}

		template<typename TOperationTraits, typename TPropertyValueTraits>
		void AssertObserverInsertsModificationValueIntoProperties(NotifyMode notifyMode) {
			// Act:
			RunTest<TOperationTraits, TPropertyValueTraits>(Num_Default_Entries + 1, notifyMode, [notifyMode](const auto&) {
				auto modificationType = NotifyMode::Commit == notifyMode ? Add : Del;
				return model::PropertyModification<typename TPropertyValueTraits::ValueType>{
					modificationType,
					TPropertyValueTraits::RandomValue()
				};
			});
		}

		template<typename TOperationTraits, typename TPropertyValueTraits>
		void AssertObserverDeletesModificationValueFromProperties(NotifyMode notifyMode) {
			// Act:
			RunTest<TOperationTraits, TPropertyValueTraits>(Num_Default_Entries - 1, notifyMode, [notifyMode](const auto& values) {
				auto modificationType = NotifyMode::Commit == notifyMode ? Del : Add;
				return model::PropertyModification<typename TPropertyValueTraits::ValueType>{ modificationType, values[2] };
			});
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TOperationTraits, typename TPropertyValueTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address_Allow) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::AllowTraits, AddressPropertyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Address_Block) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BlockTraits, AddressPropertyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic_Allow) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::AllowTraits, MosaicPropertyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic_Block) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BlockTraits, MosaicPropertyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_TransactionType_Allow) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::AllowTraits, TransactionTypePropertyTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_TransactionType_Block) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BlockTraits, TransactionTypePropertyTraits>(); \
	} \
	template<typename TOperationTraits, typename TPropertyValueTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region commit

	TRAITS_BASED_TEST(ObserverAddsAccountPropertiesInModeCommit) {
		// Act: since cache is empty there is no account property for the provided key
		AssertObserverAddsAccountProperties<TOperationTraits, TPropertyValueTraits>(NotifyMode::Commit);
	}

	TRAITS_BASED_TEST(ObserverRemovesEmptyAccountPropertiesInModeCommit) {
		// Act: cache is empty and entry is added but also removed at the end since property is empty
		AssertObserverRemovesEmptyAccountProperties<TOperationTraits, TPropertyValueTraits>(NotifyMode::Commit);
	}

	TRAITS_BASED_TEST(ObserverDoesNotRemoveNonEmptyAccountPropertiesInModeCommit) {
		// Act:
		AssertObserverDoesNotRemoveNonEmptyAccountProperties<TOperationTraits>(NotifyMode::Commit);
	}

	TRAITS_BASED_TEST(ObserverInsertsAddModificationValueIntoPropertiesInModeCommit) {
		// Act:
		AssertObserverInsertsModificationValueIntoProperties<TOperationTraits, TPropertyValueTraits>(NotifyMode::Commit);
	}

	TRAITS_BASED_TEST(ObserverDeletesDelModificationValueFromPropertiesInModeCommit) {
		// Act:
		AssertObserverDeletesModificationValueFromProperties<TOperationTraits, TPropertyValueTraits>(NotifyMode::Commit);
	}

	// endregion

	// region rollback

	TRAITS_BASED_TEST(ObserverAddsAccountPropertiesInModeRollback) {
		// Act:
		AssertObserverAddsAccountProperties<TOperationTraits, TPropertyValueTraits>(NotifyMode::Rollback);
	}

	TRAITS_BASED_TEST(ObserverRemovesEmptyAccountPropertiesInModeRollback) {
		// Act: cache is empty and entry is added but also removed at the end since property is empty
		AssertObserverRemovesEmptyAccountProperties<TOperationTraits, TPropertyValueTraits>(NotifyMode::Rollback);
	}

	TRAITS_BASED_TEST(ObserverDoesNotRemoveNonEmptyAccountPropertiesInModeRollback) {
		// Act:
		AssertObserverDoesNotRemoveNonEmptyAccountProperties<TOperationTraits>(NotifyMode::Rollback);
	}

	TRAITS_BASED_TEST(ObserverDeletesAddModificationValueFromPropertiesInModeRollback) {
		// Act:
		AssertObserverDeletesModificationValueFromProperties<TOperationTraits, TPropertyValueTraits>(NotifyMode::Rollback);
	}

	TRAITS_BASED_TEST(ObserverInsertsDelModificationValueIntoPropertiesInModeRollback) {
		// Act:
		AssertObserverInsertsModificationValueIntoProperties<TOperationTraits, TPropertyValueTraits>(NotifyMode::Rollback);
	}

	// endregion
}}
