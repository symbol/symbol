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
#include "tests/test/AccountRestrictionCacheTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS AccountRestrictionValueModificationObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::AccountRestrictionCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(AccountAddressRestrictionValueModification,)
	DEFINE_COMMON_OBSERVER_TESTS(AccountMosaicRestrictionValueModification,)
	DEFINE_COMMON_OBSERVER_TESTS(AccountOperationRestrictionValueModification,)

	namespace {
		constexpr auto Add = model::AccountRestrictionModificationType::Add;
		constexpr auto Del = model::AccountRestrictionModificationType::Del;
		constexpr auto Num_Default_Entries = 5u;

		struct AccountAddressRestrictionTraits : public test::BaseAccountAddressRestrictionTraits {
			static constexpr auto CreateObserver = CreateAccountAddressRestrictionValueModificationObserver;

			using NotificationType = model::ModifyAccountAddressRestrictionValueNotification;
		};

		struct AccountMosaicRestrictionTraits : public test::BaseAccountMosaicRestrictionTraits {
			static constexpr auto CreateObserver = CreateAccountMosaicRestrictionValueModificationObserver;

			using NotificationType = model::ModifyAccountMosaicRestrictionValueNotification;
		};

		struct AccountOperationRestrictionTraits : public test::BaseAccountOperationRestrictionTraits {
			static constexpr auto CreateObserver = CreateAccountOperationRestrictionValueModificationObserver;

			using NotificationType = model::ModifyAccountOperationRestrictionValueNotification;
		};

		bool IsInsert(NotifyMode notifyMode, model::AccountRestrictionModificationType modificationType) {
			return
					(NotifyMode::Commit == notifyMode && Add == modificationType) ||
					(NotifyMode::Rollback == notifyMode && Del == modificationType);
		}

		template<typename TRestrictionValueTraits>
		void AssertCache(
				size_t expectedSize,
				const cache::AccountRestrictionCacheDelta& delta,
				NotifyMode notifyMode,
				const Key& key,
				const model::AccountRestrictionModification<typename TRestrictionValueTraits::ValueType>& modification,
				bool shouldContainCacheEntry) {
			// Assert:
			using ValueType = typename TRestrictionValueTraits::ValueType;
			auto address = model::PublicKeyToAddress(key, model::NetworkIdentifier::Zero);

			auto iter = delta.find(address);
			if (!shouldContainCacheEntry) {
				EXPECT_FALSE(!!iter.tryGet());
				return;
			}

			const auto& restrictions = iter.get();
			auto typedRestriction = restrictions.template restriction<ValueType>(TRestrictionValueTraits::Restriction_Type);
			if (IsInsert(notifyMode, modification.ModificationType))
				EXPECT_TRUE(typedRestriction.contains(modification.Value));
			else
				EXPECT_FALSE(typedRestriction.contains(modification.Value));

			EXPECT_EQ(expectedSize, typedRestriction.size());
		}

		enum class CachePolicy { Populate, Empty };

		template<typename TOperationTraits, typename TRestrictionValueTraits, typename TModificationFactory>
		void RunTest(
				size_t expectedSize,
				NotifyMode notifyMode,
				size_t numInitialValues,
				bool shouldContainCacheEntry,
				TModificationFactory modificationFactory) {
			// Arrange:
			ObserverTestContext context(notifyMode);
			auto values = test::GenerateUniqueRandomDataVector<typename TRestrictionValueTraits::ValueType>(numInitialValues);
			auto key = test::GenerateRandomByteArray<Key>();
			test::PopulateCache<TRestrictionValueTraits, TOperationTraits>(context.cache(), key, values);

			auto modification = modificationFactory(values);
			auto unresolvedModification = model::AccountRestrictionModification<typename TRestrictionValueTraits::UnresolvedValueType>{
				modification.ModificationType,
				TRestrictionValueTraits::Unresolve(modification.Value)
			};

			auto notification = test::CreateNotification<TRestrictionValueTraits, TOperationTraits>(key, unresolvedModification);
			auto pObserver = TRestrictionValueTraits::CreateObserver();

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			AssertCache<TRestrictionValueTraits>(
					expectedSize,
					context.cache().sub<cache::AccountRestrictionCache>(),
					notifyMode,
					key,
					modification,
					shouldContainCacheEntry);
		}

		template<typename TOperationTraits, typename TRestrictionValueTraits, typename TModificationFactory>
		void RunTest(size_t expectedSize, NotifyMode notifyMode, TModificationFactory modificationFactory) {
			// Act:
			RunTest<TOperationTraits, TRestrictionValueTraits>(expectedSize, notifyMode, Num_Default_Entries, true, modificationFactory);
		}

		template<typename TOperationTraits>
		void AssertObserverDoesNotRemoveAccountRestrictionsWithValues(NotifyMode notifyMode) {
			// Arrange:
			ObserverTestContext context(notifyMode);
			auto filteredAddress = test::GenerateRandomByteArray<Address>();
			auto unresolvedFilteredAddress = AccountAddressRestrictionTraits::Unresolve(filteredAddress);
			auto key = test::GenerateRandomByteArray<Key>();
			auto& restrictionCacheDelta = context.cache().sub<cache::AccountRestrictionCache>();
			auto accountAddress = model::PublicKeyToAddress(key, model::NetworkIdentifier::Zero);
			restrictionCacheDelta.insert(state::AccountRestrictions(accountAddress));

			{
				auto& restrictions = restrictionCacheDelta.find(accountAddress).get();
				TOperationTraits::Add(restrictions.restriction(model::AccountRestrictionType::Address), state::ToVector(filteredAddress));
				TOperationTraits::Add(
						restrictions.restriction(model::AccountRestrictionType::MosaicId),
						test::GenerateRandomVector(sizeof(MosaicId)));
			}

			auto modificationType = NotifyMode::Commit == notifyMode ? Del : Add;
			auto modification = model::AccountRestrictionModification<UnresolvedAddress>{ modificationType, unresolvedFilteredAddress };
			auto completeType = TOperationTraits::CompleteAccountRestrictionType(model::AccountRestrictionType::Address);
			model::ModifyAccountAddressRestrictionValueNotification notification{ key, completeType, modification };
			auto pObserver = CreateAccountAddressRestrictionValueModificationObserver();

			// Act: cache entry is not removed, account address restriction is empty but account mosaic restriction is not
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			auto iter = context.cache().sub<cache::AccountRestrictionCache>().find(accountAddress);
			ASSERT_TRUE(!!iter.tryGet());

			const auto& restrictions = iter.get();
			EXPECT_FALSE(restrictions.isEmpty());
			EXPECT_TRUE(restrictions.restriction(model::AccountRestrictionType::Address).values().empty());
			EXPECT_EQ(1u, restrictions.restriction(model::AccountRestrictionType::MosaicId).values().size());
		}

		template<typename TOperationTraits, typename TRestrictionValueTraits>
		void AssertObserverAddsAccountRestrictions(NotifyMode notifyMode) {
			// Act:
			RunTest<TOperationTraits, TRestrictionValueTraits>(1, notifyMode, 0, true, [notifyMode](const auto&) {
				using AccountRestrictionModification = model::AccountRestrictionModification<typename TRestrictionValueTraits::ValueType>;
				auto modificationType = NotifyMode::Commit == notifyMode ? Add : Del;
				return AccountRestrictionModification{ modificationType, TRestrictionValueTraits::RandomValue() };
			});
		}

		template<typename TOperationTraits, typename TRestrictionValueTraits>
		void AssertObserverRemovesEmptyAccountRestrictions(NotifyMode notifyMode) {
			// Act:
			RunTest<TOperationTraits, TRestrictionValueTraits>(0, notifyMode, 1, false, [notifyMode](const auto& values) {
				using AccountRestrictionModification = model::AccountRestrictionModification<typename TRestrictionValueTraits::ValueType>;
				auto modificationType = NotifyMode::Commit == notifyMode ? Del : Add;
				return AccountRestrictionModification{ modificationType, values[0] };
			});
		}

		template<typename TOperationTraits, typename TRestrictionValueTraits>
		void AssertObserverInsertsModificationValueIntoRestrictions(NotifyMode notifyMode) {
			// Act:
			RunTest<TOperationTraits, TRestrictionValueTraits>(Num_Default_Entries + 1, notifyMode, [notifyMode](const auto& values) {
				using AccountRestrictionModification = model::AccountRestrictionModification<typename TRestrictionValueTraits::ValueType>;
				auto modificationType = NotifyMode::Commit == notifyMode ? Add : Del;
				return AccountRestrictionModification{ modificationType, test::CreateRandomUniqueValue(values) };
			});
		}

		template<typename TOperationTraits, typename TRestrictionValueTraits>
		void AssertObserverDeletesModificationValueFromRestrictions(NotifyMode notifyMode) {
			// Act:
			RunTest<TOperationTraits, TRestrictionValueTraits>(Num_Default_Entries - 1, notifyMode, [notifyMode](const auto& values) {
				using AccountRestrictionModification = model::AccountRestrictionModification<typename TRestrictionValueTraits::ValueType>;
				auto modificationType = NotifyMode::Commit == notifyMode ? Del : Add;
				return AccountRestrictionModification{ modificationType, values[2] };
			});
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TOperationTraits, typename TRestrictionValueTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address_Allow) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::AllowTraits, AccountAddressRestrictionTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Address_Block) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BlockTraits, AccountAddressRestrictionTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic_Allow) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::AllowTraits, AccountMosaicRestrictionTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic_Block) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BlockTraits, AccountMosaicRestrictionTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Operation_Allow) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::AllowTraits, AccountOperationRestrictionTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Operation_Block) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BlockTraits, AccountOperationRestrictionTraits>(); \
	} \
	template<typename TOperationTraits, typename TRestrictionValueTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region commit

	TRAITS_BASED_TEST(ObserverAddsAccountRestrictionsInModeCommit) {
		// Act: since cache is empty there is no account restriction for the provided key
		AssertObserverAddsAccountRestrictions<TOperationTraits, TRestrictionValueTraits>(NotifyMode::Commit);
	}

	TRAITS_BASED_TEST(ObserverRemovesEmptyAccountRestrictionsInModeCommit) {
		// Act: cache is empty and entry is added but also removed at the end since restriction is empty
		AssertObserverRemovesEmptyAccountRestrictions<TOperationTraits, TRestrictionValueTraits>(NotifyMode::Commit);
	}

	TRAITS_BASED_TEST(ObserverDoesNotRemoveAccountRestrictionsWithValuesInModeCommit) {
		// Act:
		AssertObserverDoesNotRemoveAccountRestrictionsWithValues<TOperationTraits>(NotifyMode::Commit);
	}

	TRAITS_BASED_TEST(ObserverInsertsAddModificationValueIntoRestrictionsInModeCommit) {
		// Act:
		AssertObserverInsertsModificationValueIntoRestrictions<TOperationTraits, TRestrictionValueTraits>(NotifyMode::Commit);
	}

	TRAITS_BASED_TEST(ObserverDeletesDelModificationValueFromRestrictionsInModeCommit) {
		// Act:
		AssertObserverDeletesModificationValueFromRestrictions<TOperationTraits, TRestrictionValueTraits>(NotifyMode::Commit);
	}

	// endregion

	// region rollback

	TRAITS_BASED_TEST(ObserverAddsAccountRestrictionsInModeRollback) {
		// Act:
		AssertObserverAddsAccountRestrictions<TOperationTraits, TRestrictionValueTraits>(NotifyMode::Rollback);
	}

	TRAITS_BASED_TEST(ObserverRemovesEmptyAccountRestrictionsInModeRollback) {
		// Act: cache is empty and entry is added but also removed at the end since restriction is empty
		AssertObserverRemovesEmptyAccountRestrictions<TOperationTraits, TRestrictionValueTraits>(NotifyMode::Rollback);
	}

	TRAITS_BASED_TEST(ObserverDoesNotRemoveAccountRestrictionsWithValuesInModeRollback) {
		// Act:
		AssertObserverDoesNotRemoveAccountRestrictionsWithValues<TOperationTraits>(NotifyMode::Rollback);
	}

	TRAITS_BASED_TEST(ObserverDeletesAddModificationValueFromRestrictionsInModeRollback) {
		// Act:
		AssertObserverDeletesModificationValueFromRestrictions<TOperationTraits, TRestrictionValueTraits>(NotifyMode::Rollback);
	}

	TRAITS_BASED_TEST(ObserverInsertsDelModificationValueIntoRestrictionsInModeRollback) {
		// Act:
		AssertObserverInsertsModificationValueIntoRestrictions<TOperationTraits, TRestrictionValueTraits>(NotifyMode::Rollback);
	}

	// endregion
}}
