/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "tests/test/AccountRestrictionCacheTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS AccountRestrictionValueModificationObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::AccountRestrictionCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(AccountAddressRestrictionValueModification, Height())
	DEFINE_COMMON_OBSERVER_TESTS(AccountMosaicRestrictionValueModification, Height())
	DEFINE_COMMON_OBSERVER_TESTS(AccountOperationRestrictionValueModification, Height())

	namespace {
		constexpr auto Add = model::AccountRestrictionModificationAction::Add;
		constexpr auto Del = model::AccountRestrictionModificationAction::Del;
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

		bool IsInsert(NotifyMode notifyMode, model::AccountRestrictionModificationAction modificationAction) {
			return
					(NotifyMode::Commit == notifyMode && Add == modificationAction) ||
					(NotifyMode::Rollback == notifyMode && Del == modificationAction);
		}

		template<typename TRestrictionValueTraits>
		void AssertCache(
				size_t expectedSize,
				const cache::AccountRestrictionCacheDelta& delta,
				NotifyMode notifyMode,
				const Address& address,
				const typename TRestrictionValueTraits::ValueType& restrictionValue,
				model::AccountRestrictionModificationAction action,
				bool shouldContainCacheEntry) {
			// Assert:
			auto iter = delta.find(address);
			if (!shouldContainCacheEntry) {
				EXPECT_FALSE(!!iter.tryGet());
				return;
			}

			const auto& restrictions = iter.get();
			EXPECT_EQ(2u, restrictions.version());

			const auto& restriction = restrictions.restriction(TRestrictionValueTraits::Restriction_Flags);
			if (IsInsert(notifyMode, action))
				EXPECT_TRUE(restriction.contains(state::ToVector(restrictionValue)));
			else
				EXPECT_FALSE(restriction.contains(state::ToVector(restrictionValue)));

			EXPECT_EQ(expectedSize, restriction.values().size());
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
			auto address = test::GenerateRandomByteArray<Address>();
			test::PopulateCache<TRestrictionValueTraits, TOperationTraits>(context.cache(), address, values);

			auto modification = modificationFactory(values);
			auto unresolvedRestrictionValue = TRestrictionValueTraits::Unresolve(modification.second);
			auto notification = test::CreateAccountRestrictionValueNotification<TRestrictionValueTraits, TOperationTraits>(
					address,
					unresolvedRestrictionValue,
					modification.first);
			auto pObserver = TRestrictionValueTraits::CreateObserver(Height());

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			AssertCache<TRestrictionValueTraits>(
					expectedSize,
					context.cache().sub<cache::AccountRestrictionCache>(),
					notifyMode,
					address,
					modification.second,
					modification.first,
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
			auto address = test::GenerateRandomByteArray<Address>();
			auto& restrictionCacheDelta = context.cache().sub<cache::AccountRestrictionCache>();
			restrictionCacheDelta.insert(state::AccountRestrictions(address));

			{
				auto& restrictions = restrictionCacheDelta.find(address).get();
				TOperationTraits::Add(restrictions.restriction(model::AccountRestrictionFlags::Address), state::ToVector(filteredAddress));
				TOperationTraits::Add(
						restrictions.restriction(model::AccountRestrictionFlags::MosaicId),
						test::GenerateRandomVector(sizeof(MosaicId)));
			}

			model::ModifyAccountAddressRestrictionValueNotification notification(
					address,
					TOperationTraits::CompleteAccountRestrictionFlags(model::AccountRestrictionFlags::Address),
					unresolvedFilteredAddress,
					NotifyMode::Commit == notifyMode ? Del : Add);
			auto pObserver = CreateAccountAddressRestrictionValueModificationObserver(Height());

			// Act: cache entry is not removed, account address restriction is empty but account mosaic restriction is not
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			auto iter = context.cache().sub<cache::AccountRestrictionCache>().find(address);
			ASSERT_TRUE(!!iter.tryGet());

			const auto& restrictions = iter.get();
			EXPECT_EQ(2u, restrictions.version());

			EXPECT_FALSE(restrictions.isEmpty());
			EXPECT_TRUE(restrictions.restriction(model::AccountRestrictionFlags::Address).values().empty());
			EXPECT_EQ(1u, restrictions.restriction(model::AccountRestrictionFlags::MosaicId).values().size());
		}

		template<typename TOperationTraits, typename TRestrictionValueTraits>
		void AssertObserverAddsAccountRestrictions(NotifyMode notifyMode) {
			// Act:
			RunTest<TOperationTraits, TRestrictionValueTraits>(1, notifyMode, 0, true, [notifyMode](const auto&) {
				auto modificationAction = NotifyMode::Commit == notifyMode ? Add : Del;
				return std::make_pair(modificationAction, TRestrictionValueTraits::RandomValue());
			});
		}

		template<typename TOperationTraits, typename TRestrictionValueTraits>
		void AssertObserverRemovesEmptyAccountRestrictions(NotifyMode notifyMode) {
			// Act:
			RunTest<TOperationTraits, TRestrictionValueTraits>(0, notifyMode, 1, false, [notifyMode](const auto& values) {
				auto modificationAction = NotifyMode::Commit == notifyMode ? Del : Add;
				return std::make_pair(modificationAction, values[0]);
			});
		}

		template<typename TOperationTraits, typename TRestrictionValueTraits>
		void AssertObserverInsertsModificationValueIntoRestrictions(NotifyMode notifyMode) {
			// Act:
			RunTest<TOperationTraits, TRestrictionValueTraits>(Num_Default_Entries + 1, notifyMode, [notifyMode](const auto& values) {
				auto modificationAction = NotifyMode::Commit == notifyMode ? Add : Del;
				return std::make_pair(modificationAction, test::CreateRandomUniqueValue(values));
			});
		}

		template<typename TOperationTraits, typename TRestrictionValueTraits>
		void AssertObserverDeletesModificationValueFromRestrictions(NotifyMode notifyMode) {
			// Act:
			RunTest<TOperationTraits, TRestrictionValueTraits>(Num_Default_Entries - 1, notifyMode, [notifyMode](const auto& values) {
				auto modificationAction = NotifyMode::Commit == notifyMode ? Del : Add;
				return std::make_pair(modificationAction, values[2]);
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
		// Act: since cache is empty there is no account restriction for the provided address
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

	// region serialization version fork

	namespace {
		template<typename TOperationTraits, typename TRestrictionValueTraits>
		void RunSerializationVersionForkTest(Height forkHeight, Height contextHeight, uint16_t expectedVersion) {
			// Arrange:
			ObserverTestContext context(NotifyMode::Commit, contextHeight);
			auto values = test::GenerateUniqueRandomDataVector<typename TRestrictionValueTraits::ValueType>(1);
			auto address = test::GenerateRandomByteArray<Address>();
			test::PopulateCache<TRestrictionValueTraits, TOperationTraits>(context.cache(), address, values);

			auto modification = std::make_pair(Add, TRestrictionValueTraits::RandomValue());
			auto unresolvedRestrictionValue = TRestrictionValueTraits::Unresolve(modification.second);
			auto notification = test::CreateAccountRestrictionValueNotification<TRestrictionValueTraits, TOperationTraits>(
					address,
					unresolvedRestrictionValue,
					modification.first);
			auto pObserver = TRestrictionValueTraits::CreateObserver(forkHeight);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			const auto& delta = context.cache().sub<cache::AccountRestrictionCache>();
			auto iter = delta.find(address);

			const auto& restrictions = iter.get();
			EXPECT_EQ(expectedVersion, restrictions.version()) << "forkHeight = " << forkHeight << ", contextHeight = " << contextHeight;
		}
	}

	TRAITS_BASED_TEST(RestrictionsHasForkDependentVersion) {
		RunSerializationVersionForkTest<TOperationTraits, TRestrictionValueTraits>(Height(333), Height(200), 1);
		RunSerializationVersionForkTest<TOperationTraits, TRestrictionValueTraits>(Height(333), Height(332), 1);
		RunSerializationVersionForkTest<TOperationTraits, TRestrictionValueTraits>(Height(333), Height(333), 1);
		RunSerializationVersionForkTest<TOperationTraits, TRestrictionValueTraits>(Height(333), Height(334), 2);
		RunSerializationVersionForkTest<TOperationTraits, TRestrictionValueTraits>(Height(333), Height(400), 2);
	}

	// endregion
}}
