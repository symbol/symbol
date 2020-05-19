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
#include "tests/test/AliasTestUtils.h"
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS AliasedDataObserversTests

	using ObserverTestContext = test::ObserverTestContextT<test::NamespaceCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(AliasedAddress,)
	DEFINE_COMMON_OBSERVER_TESTS(AliasedMosaicId,)

	// region traits

	namespace {
		struct CommitTraits {
			static constexpr auto Notify_Mode = NotifyMode::Commit;
			static constexpr auto Create_Link = model::AliasAction::Link;
			static constexpr auto Remove_Link = model::AliasAction::Unlink;
		};

		struct RollbackTraits {
			static constexpr auto Notify_Mode = NotifyMode::Rollback;
			// during rollback actions need to be reversed to create or remove link
			static constexpr auto Create_Link = model::AliasAction::Unlink;
			static constexpr auto Remove_Link = model::AliasAction::Link;
		};

		struct AddressTraits {
			using NotificationType = model::AliasedAddressNotification;
			using AliasType = Address;

			static auto CreateObserver() {
				return CreateAliasedAddressObserver();
			}
		};

		struct MosaicIdTraits {
			using NotificationType = model::AliasedMosaicIdNotification;
			using AliasType = MosaicId;

			static auto CreateObserver() {
				return CreateAliasedMosaicIdObserver();
			}
		};
	}

#define MAKE_ALIASED_DATA_OBSERVER_TEST(TEST_NAME) \
	template<typename TTraits, typename TDirectionTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Commit_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits, CommitTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Commit_MosaicId) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicIdTraits, CommitTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Rollback_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits, RollbackTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Rollback_MosaicId) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicIdTraits, RollbackTraits>(); } \
	template<typename TTraits, typename TDirectionTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region test utils

	namespace {
		static constexpr auto Default_Namespace_Id = NamespaceId(123);

		template<typename TTraits, typename TSeedCacheFunc, typename TCheckCacheFunc>
		void RunTest(
				const typename TTraits::NotificationType& notification,
				ObserverTestContext&& context,
				TSeedCacheFunc seedCache,
				TCheckCacheFunc checkCache) {
			// Arrange:
			auto pObserver = TTraits::CreateObserver();

			// - seed the cache
			auto& namespaceCacheDelta = context.cache().sub<cache::NamespaceCache>();
			seedCache(namespaceCacheDelta);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert: check the cache
			checkCache(namespaceCacheDelta, notification.NamespaceId);
		}

		template<typename TTraits>
		typename TTraits::NotificationType CreateNotification(model::AliasAction aliasAction) {
			typename TTraits::AliasType alias;
			test::FillWithRandomData(alias);
			return typename TTraits::NotificationType(Default_Namespace_Id, aliasAction, alias);
		}

		void SeedCacheWithoutLink(cache::NamespaceCacheDelta& namespaceCacheDelta) {
			auto owner = test::CreateRandomOwner();
			namespaceCacheDelta.insert(state::RootNamespace(Default_Namespace_Id, owner, test::CreateLifetime(10, 20)));
		}

		template<typename TTraits>
		void SeedCacheWithLink(cache::NamespaceCacheDelta& namespaceCacheDelta) {
			SeedCacheWithoutLink(namespaceCacheDelta);
			test::SetRandomAlias<typename TTraits::AliasType>(namespaceCacheDelta, Default_Namespace_Id);
		}
	}

	// endregion

	// region create link

	MAKE_ALIASED_DATA_OBSERVER_TEST(ObserverCreatesLink) {
		// Arrange:
		auto notification = CreateNotification<TTraits>(TDirectionTraits::Create_Link);

		// Act:
		RunTest<TTraits>(
				notification,
				ObserverTestContext(TDirectionTraits::Notify_Mode, Height(888)),
				SeedCacheWithoutLink,
				[&notification](const auto& namespaceCacheDelta, auto namespaceId) {
					// Assert: validate alias that namespace should have
					auto namespaceIter = namespaceCacheDelta.find(namespaceId);
					ASSERT_TRUE(!!namespaceIter.tryGet());

					const auto& entry = namespaceIter.get();
					test::AssertEqualAlias(state::NamespaceAlias(notification.AliasedData), entry.root().alias(namespaceId));
				});
	}

	// endregion

	// region remove link

	MAKE_ALIASED_DATA_OBSERVER_TEST(ObserverRemovesLink) {
		// Arrange:
		auto notification = CreateNotification<TTraits>(TDirectionTraits::Remove_Link);

		// Act:
		RunTest<TTraits>(
				notification,
				ObserverTestContext(TDirectionTraits::Notify_Mode, Height(888)),
				SeedCacheWithLink<TTraits>,
				[](const auto& namespaceCacheDelta, auto namespaceId) {
					// Assert: alias does not exist anymore
					auto namespaceIter = namespaceCacheDelta.find(namespaceId);
					ASSERT_TRUE(!!namespaceIter.tryGet());

					const auto& entry = namespaceIter.get();
					auto aliasType = entry.root().alias(namespaceId).type();
					EXPECT_EQ(state::AliasType::None, aliasType);
				});
	}

	// endregion
}}
