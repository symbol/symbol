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

#define TEST_CLASS RootNamespaceObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(RootNamespace,)

	namespace {
		constexpr auto Grace_Period_Duration = 7u;

		class ObserverTestContext : public test::ObserverTestContextT<test::NamespaceCacheFactory> {
		public:
			explicit ObserverTestContext(observers::NotifyMode mode) : ObserverTestContext(mode, Height(444))
			{}

			ObserverTestContext(observers::NotifyMode mode, Height height)
					: test::ObserverTestContextT<test::NamespaceCacheFactory>(mode, height, CreateConfiguration())
			{}

		private:
			static model::BlockChainConfiguration CreateConfiguration() {
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.Plugins.emplace("namespace::ex", utils::ConfigurationBag({
					{
						"",
						{
							{ "gracePeriodDuration", "7" }
						}
					}
				}));
				return config;
			}
		};

		model::RootNamespaceNotification CreateRootNotification(const Address& owner, NamespaceId id) {
			return model::RootNamespaceNotification(owner, id, BlockDuration());
		}

		template<typename TSeedCacheFunc, typename TCheckCacheFunc>
		void RunRootTest(
				const model::RootNamespaceNotification& notification,
				ObserverTestContext&& context,
				TSeedCacheFunc seedCache,
				TCheckCacheFunc checkCache) {
			// Arrange:
			auto pObserver = CreateRootNamespaceObserver();

			// - seed the cache
			auto& namespaceCacheDelta = context.cache().sub<cache::NamespaceCache>();
			seedCache(namespaceCacheDelta);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert: check the cache
			checkCache(namespaceCacheDelta);
		}

		void SeedCacheEmpty(cache::NamespaceCacheDelta& namespaceCacheDelta) {
			// Sanity:
			test::AssertCacheContents(namespaceCacheDelta, {});
		}

		auto SeedCacheWithRoot25TreeOwner(const Address& owner) {
			return [owner](auto& namespaceCacheDelta) {
				// Arrange: create a cache with { 25 } and { 25, 36 }
				auto lifetime = test::CreateLifetime(10, 123 + Grace_Period_Duration);
				namespaceCacheDelta.insert(state::RootNamespace(NamespaceId(25), owner, lifetime));
				namespaceCacheDelta.insert(state::Namespace(test::CreatePath({ 25, 36 })));

				// Sanity:
				test::AssertCacheContents(namespaceCacheDelta, { 25, 36 });
			};
		}
	}

	// region commit

	namespace {
		void AssertRootAdded(
				const cache::NamespaceCacheDelta& namespaceCacheDelta,
				const Address& owner,
				Height lifetimeStart,
				Height lifetimeEnd) {
			// Assert: the root was added
			EXPECT_EQ(1u, namespaceCacheDelta.activeSize());
			ASSERT_TRUE(namespaceCacheDelta.contains(NamespaceId(25)));

			auto namespaceIter = namespaceCacheDelta.find(NamespaceId(25));
			const auto& entry = namespaceIter.get();
			EXPECT_EQ(Namespace_Base_Id, entry.ns().parentId());

			EXPECT_EQ(owner, entry.root().ownerAddress());
			EXPECT_EQ(lifetimeStart, entry.root().lifetime().Start);
			EXPECT_EQ(lifetimeEnd, entry.root().lifetime().End);
			EXPECT_TRUE(entry.root().empty());
		}
	}

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_Root) {
		// Arrange: create a new root namespace with a finite duration
		auto owner = test::CreateRandomOwner();
		auto notification = CreateRootNotification(owner, NamespaceId(25));
		notification.Duration = BlockDuration(1100);

		// Act: add it
		auto seedCache = SeedCacheEmpty;
		RunRootTest(notification, ObserverTestContext(NotifyMode::Commit, Height(777)), seedCache, [&owner](auto& namespaceCacheDelta) {
			// Assert: the root was added
			AssertRootAdded(namespaceCacheDelta, owner, Height(777), Height(1877 + Grace_Period_Duration));
		});
	}

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_RootEternal) {
		// Arrange: create a new root namespace with an eternal duration
		auto owner = test::CreateRandomOwner();
		auto notification = CreateRootNotification(owner, NamespaceId(25));
		notification.Duration = Eternal_Artifact_Duration;

		// Act: add it (note that adding eternal namespaces after nemesis is prevented by a validator)
		auto seedCache = SeedCacheEmpty;
		RunRootTest(notification, ObserverTestContext(NotifyMode::Commit, Height(777)), seedCache, [&owner](auto& namespaceCacheDelta) {
			// Assert: the root was added
			AssertRootAdded(namespaceCacheDelta, owner, Height(777), Height(0xFFFF'FFFF'FFFF'FFFF));
		});
	}

	namespace {
		void AssertRootRenewed(
				cache::NamespaceCacheDelta& namespaceCacheDelta,
				const Address& owner,
				Height lifetimeStart,
				Height lifetimeEnd) {
			// Assert: the root was renewed
			EXPECT_EQ(2u, namespaceCacheDelta.activeSize());
			EXPECT_EQ(4u, namespaceCacheDelta.deepSize());
			ASSERT_TRUE(namespaceCacheDelta.contains(NamespaceId(25)));

			auto namespaceIter = namespaceCacheDelta.find(NamespaceId(25));
			const auto& entry = namespaceIter.get();
			EXPECT_EQ(Namespace_Base_Id, entry.ns().parentId());

			EXPECT_EQ(owner, entry.root().ownerAddress());
			EXPECT_EQ(lifetimeStart, entry.root().lifetime().Start);
			EXPECT_EQ(lifetimeEnd, entry.root().lifetime().End);
			EXPECT_EQ(1u, entry.root().size());
		}
	}

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_RootRenewalSameOwnerActive) {
		// Arrange:
		for (auto height : { Height(75), Height(122), Height(122 + Grace_Period_Duration) }) {
			// - create a root namespace with a finite duration
			auto owner = test::CreateRandomOwner();
			auto notification = CreateRootNotification(owner, NamespaceId(25));
			notification.Duration = BlockDuration(1100);

			// Act: renew it before its expiry
			auto seedCache = SeedCacheWithRoot25TreeOwner(owner);
			RunRootTest(notification, ObserverTestContext(NotifyMode::Commit, height), seedCache, [&owner](auto& namespaceCacheDelta) {
				// Assert: the root was renewed - [initial start, initial end + duration)
				AssertRootRenewed(namespaceCacheDelta, owner, Height(10), Height(1223 + Grace_Period_Duration));
			});
		}
	}

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_RootRenewalSameOwnerInactive) {
		// Arrange:
		for (auto height : { Height(123 + Grace_Period_Duration), Height(250) }) {
			// - create a root namespace with a finite duration
			auto owner = test::CreateRandomOwner();
			auto notification = CreateRootNotification(owner, NamespaceId(25));
			notification.Duration = BlockDuration(1100);

			// Act: renew it after its expiry
			auto testContext = ObserverTestContext(NotifyMode::Commit, height);
			auto seedCache = SeedCacheWithRoot25TreeOwner(owner);
			RunRootTest(notification, std::move(testContext), seedCache, [height, &owner](auto& namespaceCacheDelta) {
				// Assert: the root was renewed - [block height, block height + duration)
				AssertRootRenewed(namespaceCacheDelta, owner, height, height + Height(1100 + Grace_Period_Duration));
			});
		}
	}

	namespace {
		void AssertRootOwnerChanged(
				cache::NamespaceCacheDelta& namespaceCacheDelta,
				const Address& owner,
				Height lifetimeStart,
				Height lifetimeEnd) {
			// Assert: the root was renewed
			EXPECT_EQ(1u, namespaceCacheDelta.activeSize());
			EXPECT_EQ(3u, namespaceCacheDelta.deepSize());
			ASSERT_TRUE(namespaceCacheDelta.contains(NamespaceId(25)));

			auto namespaceIter = namespaceCacheDelta.find(NamespaceId(25));
			const auto& entry = namespaceIter.get();
			EXPECT_EQ(Namespace_Base_Id, entry.ns().parentId());

			EXPECT_EQ(owner, entry.root().ownerAddress());
			EXPECT_EQ(lifetimeStart, entry.root().lifetime().Start);
			EXPECT_EQ(lifetimeEnd, entry.root().lifetime().End);
			EXPECT_TRUE(entry.root().empty());
		}
	}

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_RootRenewalChangeOwnerActive) {
		// Arrange:
		for (auto height : { Height(75), Height(122), Height(122 + Grace_Period_Duration) }) {
			// - create a root namespace with a finite duration
			auto owner = test::CreateRandomOwner();
			auto notification = CreateRootNotification(owner, NamespaceId(25));
			notification.Duration = BlockDuration(1100);

			// Act: change owner before its expiry (this is prevented by a validator)
			auto testContext = ObserverTestContext(NotifyMode::Commit, height);
			auto seedCache = SeedCacheWithRoot25TreeOwner(test::CreateRandomOwner());
			RunRootTest(notification, std::move(testContext), seedCache, [height, &owner](auto& namespaceCacheDelta) {
				// Assert: the root was renewed - [block height, block height + duration)
				AssertRootOwnerChanged(namespaceCacheDelta, owner, height, height + Height(1100 + Grace_Period_Duration));
			});
		}
	}

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_RootRenewalChangeOwnerInactive) {
		// Arrange:
		for (auto height : { Height(123 + Grace_Period_Duration), Height(250) }) {
			// - create a root namespace with a finite duration
			auto owner = test::CreateRandomOwner();
			auto notification = CreateRootNotification(owner, NamespaceId(25));
			notification.Duration = BlockDuration(1100);

			// Act: change owner after its expiry
			auto testContext = ObserverTestContext(NotifyMode::Commit, height);
			auto seedCache = SeedCacheWithRoot25TreeOwner(test::CreateRandomOwner());
			RunRootTest(notification, std::move(testContext), seedCache, [height, &owner](auto& namespaceCacheDelta) {
				// Assert: the root was renewed - [block height, block height + duration)
				AssertRootOwnerChanged(namespaceCacheDelta, owner, height, height + Height(1100 + Grace_Period_Duration));
			});
		}
	}

	// endregion

	// region rollback

	TEST(TEST_CLASS, ObserverRemovesNamespaceOnRollback_Root) {
		// Arrange: create a root namespace for removal
		auto owner = test::CreateRandomOwner();
		auto notification = CreateRootNotification(owner, NamespaceId(25));

		// Act: remove it
		RunRootTest(
				notification,
				ObserverTestContext(NotifyMode::Rollback),
				[&owner](auto& namespaceCacheDelta) {
					// Arrange: create a cache with { 25, 26 }
					namespaceCacheDelta.insert(state::RootNamespace(NamespaceId(25), owner, test::CreateLifetime(10, 20)));
					namespaceCacheDelta.insert(state::RootNamespace(NamespaceId(26), owner, test::CreateLifetime(10, 20)));

					// Sanity:
					test::AssertCacheContents(namespaceCacheDelta, { 25, 26 });
				},
				[](auto& namespaceCacheDelta) {
					// Assert: the root was removed
					EXPECT_EQ(1u, namespaceCacheDelta.activeSize());
					EXPECT_FALSE(namespaceCacheDelta.contains(NamespaceId(25)));
					EXPECT_TRUE(namespaceCacheDelta.contains(NamespaceId(26)));
				});
	}

	// endregion
}}
