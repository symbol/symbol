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
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS ChildNamespaceObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::NamespaceCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(ChildNamespace,)

	namespace {
		model::ChildNamespaceNotification CreateChildNotification(const Key& signer, NamespaceId parentId, NamespaceId id) {
			return model::ChildNamespaceNotification(signer, id, parentId);
		}

		template<typename TSeedCacheFunc, typename TCheckCacheFunc>
		void RunChildTest(
				const model::ChildNamespaceNotification& notification,
				ObserverTestContext&& context,
				TSeedCacheFunc seedCache,
				TCheckCacheFunc checkCache) {
			// Arrange:
			auto pObserver = CreateChildNamespaceObserver();

			// - seed the cache
			auto& namespaceCacheDelta = context.cache().sub<cache::NamespaceCache>();
			seedCache(namespaceCacheDelta);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert: check the cache
			checkCache(namespaceCacheDelta);
		}

		auto SeedCacheWithRoot25TreeSigner(const Key& signer) {
			return [&signer](auto& namespaceCacheDelta) {
				// Arrange: create a cache with { 25 } and { 25, 36 }
				namespaceCacheDelta.insert(state::RootNamespace(NamespaceId(25), signer, test::CreateLifetime(10, 123)));
				namespaceCacheDelta.insert(state::Namespace(test::CreatePath({ 25, 36 })));

				// Sanity:
				test::AssertCacheContents(namespaceCacheDelta, { 25, 36 });
			};
		}
	}

	// region commit

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_ChildRootParent) {
		// Arrange: create a child namespace with a root parent
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateChildNotification(signer, NamespaceId(25), NamespaceId(37));

		// Act: add it
		RunChildTest(
				notification,
				ObserverTestContext(NotifyMode::Commit),
				SeedCacheWithRoot25TreeSigner(signer),
				[](auto& namespaceCacheDelta) {
					// Assert: the child was added
					EXPECT_EQ(3u, namespaceCacheDelta.activeSize());
					EXPECT_TRUE(namespaceCacheDelta.contains(NamespaceId(25)));
					EXPECT_TRUE(namespaceCacheDelta.contains(NamespaceId(36)));

					ASSERT_TRUE(namespaceCacheDelta.contains(NamespaceId(37)));
					EXPECT_EQ(NamespaceId(25), namespaceCacheDelta.get(NamespaceId(37)).ns().parentId());
				});
	}

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_ChildNonRootParent) {
		// Arrange: create a child namespace with a non-root parent
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateChildNotification(signer, NamespaceId(36), NamespaceId(49));

		// Act: add it
		RunChildTest(
				notification,
				ObserverTestContext(NotifyMode::Commit),
				SeedCacheWithRoot25TreeSigner(signer),
				[](auto& namespaceCacheDelta) {
					// Assert: the child was added
					EXPECT_EQ(3u, namespaceCacheDelta.activeSize());
					EXPECT_TRUE(namespaceCacheDelta.contains(NamespaceId(25)));
					EXPECT_TRUE(namespaceCacheDelta.contains(NamespaceId(36)));

					ASSERT_TRUE(namespaceCacheDelta.contains(NamespaceId(49)));
					EXPECT_EQ(NamespaceId(36), namespaceCacheDelta.get(NamespaceId(49)).ns().parentId());
				});
	}

	// endregion

	// region rollback

	TEST(TEST_CLASS, ObserverRemovesNamespaceOnRollback_ChildRootParent) {
		// Arrange: create a child namespace with a root parent for removal
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateChildNotification(signer, NamespaceId(25), NamespaceId(36));

		// Act: remove it
		RunChildTest(
				notification,
				ObserverTestContext(NotifyMode::Rollback),
				[&signer](auto& namespaceCacheDelta) {
					// Arrange: create a cache with { 25 } and { 25, 36 }
					namespaceCacheDelta.insert(state::RootNamespace(NamespaceId(25), signer, test::CreateLifetime(10, 20)));
					namespaceCacheDelta.insert(state::Namespace(test::CreatePath({ 25, 36 })));

					// Sanity:
					test::AssertCacheContents(namespaceCacheDelta, { 25, 36 });
				},
				[](auto& namespaceCacheDelta) {
					// Assert: the child was removed
					EXPECT_EQ(1u, namespaceCacheDelta.activeSize());
					EXPECT_TRUE(namespaceCacheDelta.contains(NamespaceId(25)));
					EXPECT_FALSE(namespaceCacheDelta.contains(NamespaceId(36)));
				});
	}

	TEST(TEST_CLASS, ObserverRemovesNamespaceOnRollback_ChildNonRootParent) {
		// Arrange: create a child namespace with a non-root parent for removal
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateChildNotification(signer, NamespaceId(36), NamespaceId(49));

		// Act: remove it
		RunChildTest(
				notification,
				ObserverTestContext(NotifyMode::Rollback),
				[&signer](auto& namespaceCacheDelta) {
					// Arrange: create a cache with { 25 }, { 25, 36 } and { 25, 36, 49 }
					namespaceCacheDelta.insert(state::RootNamespace(NamespaceId(25), signer, test::CreateLifetime(10, 20)));
					namespaceCacheDelta.insert(state::Namespace(test::CreatePath({ 25, 36 })));
					namespaceCacheDelta.insert(state::Namespace(test::CreatePath({ 25, 36, 49 })));

					// Sanity:
					test::AssertCacheContents(namespaceCacheDelta, { 25, 36, 49 });
				},
				[](auto& namespaceCacheDelta) {
					// Assert: the child was removed
					EXPECT_EQ(2u, namespaceCacheDelta.activeSize());
					EXPECT_TRUE(namespaceCacheDelta.contains(NamespaceId(25)));
					EXPECT_TRUE(namespaceCacheDelta.contains(NamespaceId(36)));
					EXPECT_FALSE(namespaceCacheDelta.contains(NamespaceId(49)));
				});
	}

	// endregion
}}
