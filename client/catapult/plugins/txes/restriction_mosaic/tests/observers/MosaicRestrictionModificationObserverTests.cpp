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
#include "tests/test/MosaicRestrictionCacheTestUtils.h"
#include "tests/test/MosaicRestrictionTestTraits.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS MosaicRestrictionModificationObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(MosaicGlobalRestrictionCommitModification,)
	DEFINE_COMMON_OBSERVER_TESTS(MosaicGlobalRestrictionRollbackModification,)

	DEFINE_COMMON_OBSERVER_TESTS(MosaicAddressRestrictionCommitModification,)
	DEFINE_COMMON_OBSERVER_TESTS(MosaicAddressRestrictionRollbackModification,)

	// region traits

	namespace {
		using ObserverTestContext = test::ObserverTestContextT<test::MosaicRestrictionCacheFactory>;

		enum class InitializationScheme { Unset, Set };

		template<typename TTestTraits>
		class BasicTraits : public TTestTraits {
		public:
			typename TTestTraits::NotificationType createNotification(uint64_t key, uint64_t value, InitializationScheme scheme) {
				if (InitializationScheme::Unset == scheme)
					return this->createDeleteNotification(key);
				else
					return this->createAddNotification(key, value);
			}

			void addRestrictionWithValueToCache(cache::MosaicRestrictionCacheDelta& restrictionCache, uint64_t key, uint64_t value) {
				this->addRestrictionWithValuesToCache(restrictionCache, { std::make_pair(key, value) });
			}

		private:
			TTestTraits m_traits;
		};

		using GlobalCommitNotification = model::MosaicGlobalRestrictionModificationNewValueNotification;
		using GlobalRollbackNotification = model::MosaicGlobalRestrictionModificationPreviousValueNotification;
		using AddressCommitNotification = model::MosaicAddressRestrictionModificationNewValueNotification;
		using AddressRollbackNotification = model::MosaicAddressRestrictionModificationPreviousValueNotification;

		class GlobalCommitTraits : public BasicTraits<test::MosaicGlobalRestrictionTestTraits<GlobalCommitNotification>> {
		public:
			static constexpr auto Execute_Mode = NotifyMode::Commit;
			static constexpr auto Bypass_Mode = NotifyMode::Rollback;

			static constexpr auto CreateObserver = CreateMosaicGlobalRestrictionCommitModificationObserver;
		};

		class GlobalRollbackTraits : public BasicTraits<test::MosaicGlobalRestrictionTestTraits<GlobalRollbackNotification>> {
		public:
			static constexpr auto Execute_Mode = NotifyMode::Rollback;
			static constexpr auto Bypass_Mode = NotifyMode::Commit;

			static constexpr auto CreateObserver = CreateMosaicGlobalRestrictionRollbackModificationObserver;
		};

		class AddressCommitTraits : public BasicTraits<test::MosaicAddressRestrictionTestTraits<AddressCommitNotification>> {
		public:
			static constexpr auto Execute_Mode = NotifyMode::Commit;
			static constexpr auto Bypass_Mode = NotifyMode::Rollback;

			static constexpr auto CreateObserver = CreateMosaicAddressRestrictionCommitModificationObserver;
		};

		class AddressRollbackTraits : public BasicTraits<test::MosaicAddressRestrictionTestTraits<AddressRollbackNotification>> {
		public:
			static constexpr auto Execute_Mode = NotifyMode::Rollback;
			static constexpr auto Bypass_Mode = NotifyMode::Commit;

			static constexpr auto CreateObserver = CreateMosaicAddressRestrictionRollbackModificationObserver;
		};
	}

#define RESTRICTION_TYPE_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(MosaicGlobalRestrictionCommitModificationObserverTests, TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<GlobalCommitTraits>(); \
	} \
	TEST(MosaicGlobalRestrictionRollbackModificationObserverTests, TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<GlobalRollbackTraits>(); \
	} \
	TEST(MosaicAddressRestrictionCommitModificationObserverTests, TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressCommitTraits>(); \
	} \
	TEST(MosaicAddressRestrictionRollbackModificationObserverTests, TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressRollbackTraits>(); \
	} \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region cache does not contain entry

	RESTRICTION_TYPE_BASED_TEST(OppositeNotificationModeBypassesChangesWhenCacheDoesNotContainEntry) {
		// Arrange:
		TTraits traits;
		ObserverTestContext context(TTraits::Bypass_Mode);
		const auto& restrictionCache = context.cache().sub<cache::MosaicRestrictionCache>();

		auto pObserver = TTraits::CreateObserver();
		auto notification = traits.createNotification(123, 456, InitializationScheme::Set);

		// Sanity:
		EXPECT_EQ(0u, restrictionCache.size());

		// Act:
		test::ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(0u, restrictionCache.size());
	}

	RESTRICTION_TYPE_BASED_TEST(CanAddNewRestrictionWhenCacheDoesNotContainEntry) {
		// Arrange:
		TTraits traits;
		ObserverTestContext context(TTraits::Execute_Mode);
		const auto& restrictionCache = context.cache().sub<cache::MosaicRestrictionCache>();

		auto pObserver = TTraits::CreateObserver();
		auto notification = traits.createNotification(123, 456, InitializationScheme::Set);

		// Sanity:
		EXPECT_EQ(0u, restrictionCache.size());

		// Act:
		test::ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(1u, restrictionCache.size());

		auto restrictionEntryIter = restrictionCache.find(traits.uniqueKey());
		ASSERT_TRUE(!!restrictionEntryIter.tryGet());
		traits.assertEqual(restrictionEntryIter.get(), { std::make_pair(123, 456) });
	}

	// endregion

	// region cache contains entry but not rule

	RESTRICTION_TYPE_BASED_TEST(OppositeNotificationModeBypassesChangesWhenCacheContainsEntryButNotRule) {
		// Arrange:
		TTraits traits;
		ObserverTestContext context(TTraits::Bypass_Mode);
		auto& restrictionCache = context.cache().sub<cache::MosaicRestrictionCache>();

		auto pObserver = TTraits::CreateObserver();
		auto notification = traits.createNotification(123, 456, InitializationScheme::Set);
		traits.addRestrictionWithValueToCache(restrictionCache, 234, 888);

		// Sanity:
		EXPECT_EQ(1u, restrictionCache.size());

		// Act:
		test::ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(1u, restrictionCache.size());

		auto restrictionEntryIter = restrictionCache.find(traits.uniqueKey());
		ASSERT_TRUE(!!restrictionEntryIter.tryGet());
		traits.assertEqual(restrictionEntryIter.get(), { std::make_pair(234, 888) });
	}

	RESTRICTION_TYPE_BASED_TEST(CanAddNewRestrictionWhenCacheContainsEntryButNotRule) {
		// Arrange:
		TTraits traits;
		ObserverTestContext context(TTraits::Execute_Mode);
		auto& restrictionCache = context.cache().sub<cache::MosaicRestrictionCache>();

		auto pObserver = TTraits::CreateObserver();
		auto notification = traits.createNotification(123, 456, InitializationScheme::Set);
		traits.addRestrictionWithValueToCache(restrictionCache, 234, 888);

		// Sanity:
		EXPECT_EQ(1u, restrictionCache.size());

		// Act:
		test::ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(1u, restrictionCache.size());

		auto restrictionEntryIter = restrictionCache.find(traits.uniqueKey());
		ASSERT_TRUE(!!restrictionEntryIter.tryGet());
		traits.assertEqual(restrictionEntryIter.get(), { std::make_pair(234, 888), std::make_pair(123, 456) });
	}

	// endregion

	// region cache contains entry and rule

	RESTRICTION_TYPE_BASED_TEST(OppositeNotificationModeBypassesChangesWhenCacheContainsEntryAndRule) {
		// Arrange:
		TTraits traits;
		ObserverTestContext context(TTraits::Bypass_Mode);
		auto& restrictionCache = context.cache().sub<cache::MosaicRestrictionCache>();

		auto pObserver = TTraits::CreateObserver();
		auto notification = traits.createNotification(123, 456, InitializationScheme::Set);
		traits.addRestrictionWithValueToCache(restrictionCache, 123, 111);

		// Sanity:
		EXPECT_EQ(1u, restrictionCache.size());

		// Act:
		test::ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(1u, restrictionCache.size());

		auto restrictionEntryIter = restrictionCache.find(traits.uniqueKey());
		ASSERT_TRUE(!!restrictionEntryIter.tryGet());
		traits.assertEqual(restrictionEntryIter.get(), { std::make_pair(123, 111) });
	}

	RESTRICTION_TYPE_BASED_TEST(CanModifyExistingRestrictionWhenCacheContainsEntryAndRule) {
		// Arrange:
		TTraits traits;
		ObserverTestContext context(TTraits::Execute_Mode);
		auto& restrictionCache = context.cache().sub<cache::MosaicRestrictionCache>();

		auto pObserver = TTraits::CreateObserver();
		auto notification = traits.createNotification(123, 456, InitializationScheme::Set);
		traits.addRestrictionWithValueToCache(restrictionCache, 123, 111);

		// Sanity:
		EXPECT_EQ(1u, restrictionCache.size());

		// Act:
		test::ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(1u, restrictionCache.size());

		auto restrictionEntryIter = restrictionCache.find(traits.uniqueKey());
		ASSERT_TRUE(!!restrictionEntryIter.tryGet());
		traits.assertEqual(restrictionEntryIter.get(), { std::make_pair(123, 456) });
	}

	RESTRICTION_TYPE_BASED_TEST(CanDeleteExistingRestrictionWhenCacheContainsEntryAndRule) {
		// Arrange:
		TTraits traits;
		ObserverTestContext context(TTraits::Execute_Mode);
		auto& restrictionCache = context.cache().sub<cache::MosaicRestrictionCache>();

		auto pObserver = TTraits::CreateObserver();
		auto notification = traits.createNotification(123, 0, InitializationScheme::Unset);
		traits.addRestrictionWithValuesToCache(restrictionCache, { std::make_pair(123, 111), std::make_pair(246, 222) });

		// Sanity:
		EXPECT_EQ(1u, restrictionCache.size());

		// Act:
		test::ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(1u, restrictionCache.size());

		auto restrictionEntryIter = restrictionCache.find(traits.uniqueKey());
		ASSERT_TRUE(!!restrictionEntryIter.tryGet());
		traits.assertEqual(restrictionEntryIter.get(), { std::make_pair(246, 222) });
	}

	RESTRICTION_TYPE_BASED_TEST(CanDeleteExistingEntryWhenCacheContainsEntryAndRule) {
		// Arrange:
		TTraits traits;
		ObserverTestContext context(TTraits::Execute_Mode);
		auto& restrictionCache = context.cache().sub<cache::MosaicRestrictionCache>();

		auto pObserver = TTraits::CreateObserver();
		auto notification = traits.createNotification(123, 0, InitializationScheme::Unset);
		traits.addRestrictionWithValueToCache(restrictionCache, 123, 111);

		// Sanity:
		EXPECT_EQ(1u, restrictionCache.size());

		// Act:
		test::ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(0u, restrictionCache.size());

		auto restrictionEntryIter = restrictionCache.find(traits.uniqueKey());
		EXPECT_FALSE(!!restrictionEntryIter.tryGet());
	}

	// endregion
}}
