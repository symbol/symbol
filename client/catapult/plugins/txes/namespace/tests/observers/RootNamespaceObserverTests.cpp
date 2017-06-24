#include "src/observers/Observers.h"
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS RootNamespaceObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::NamespaceCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(RootNamespace,)

	namespace {
		model::RootNamespaceNotification CreateRootNotification(const Key& signer, NamespaceId id) {
			return model::RootNamespaceNotification(signer, id, ArtifactDuration());
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

	namespace {
		void AssertRootAdded(
				const cache::NamespaceCacheDelta& namespaceCacheDelta,
				const Key& signer,
				Height lifetimeStart,
				Height lifetimeEnd) {
			// Assert: the root was added
			EXPECT_EQ(1u, namespaceCacheDelta.activeSize());
			ASSERT_TRUE(namespaceCacheDelta.contains(NamespaceId(25)));

			const auto& entry = namespaceCacheDelta.get(NamespaceId(25));
			EXPECT_EQ(Namespace_Base_Id, entry.ns().parentId());

			EXPECT_EQ(signer, entry.root().owner());
			EXPECT_EQ(lifetimeStart, entry.root().lifetime().Start);
			EXPECT_EQ(lifetimeEnd, entry.root().lifetime().End);
			EXPECT_TRUE(entry.root().empty());
		}
	}

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_Root) {
		// Arrange: create a new root namespace with a finite duration
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateRootNotification(signer, NamespaceId(25));
		notification.Duration = ArtifactDuration(1100);

		// Act: add it
		RunRootTest(
				notification,
				ObserverTestContext(NotifyMode::Commit, Height(777)),
				SeedCacheEmpty,
				[&signer](auto& namespaceCacheDelta) {
					// Assert: the root was added
					AssertRootAdded(namespaceCacheDelta, signer, Height(777), Height(1877));
				});
	}

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_RootEternal) {
		// Arrange: create a new root namespace with an eternal duration
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateRootNotification(signer, NamespaceId(25));
		notification.Duration = Eternal_Artifact_Duration;

		// Act: add it (note that adding eternal namespaces after nemesis is prevented by a validator)
		RunRootTest(
				notification,
				ObserverTestContext(NotifyMode::Commit, Height(777)),
				SeedCacheEmpty,
				[&signer](auto& namespaceCacheDelta) {
					// Assert: the root was added
					AssertRootAdded(namespaceCacheDelta, signer, Height(777), Height(0xFFFF'FFFF'FFFF'FFFF));
				});
	}

	namespace {
		void AssertRootRenewed(
				cache::NamespaceCacheDelta& namespaceCacheDelta,
				const Key& signer,
				Height lifetimeStart,
				Height lifetimeEnd) {
			// Assert: the root was renewed
			EXPECT_EQ(2u, namespaceCacheDelta.activeSize());
			EXPECT_EQ(4u, namespaceCacheDelta.deepSize());
			ASSERT_TRUE(namespaceCacheDelta.contains(NamespaceId(25)));

			const auto& entry = namespaceCacheDelta.get(NamespaceId(25));
			EXPECT_EQ(Namespace_Base_Id, entry.ns().parentId());

			EXPECT_EQ(signer, entry.root().owner());
			EXPECT_EQ(lifetimeStart, entry.root().lifetime().Start);
			EXPECT_EQ(lifetimeEnd, entry.root().lifetime().End);
			EXPECT_EQ(1u, entry.root().size());
		}
	}

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_RootRenewalSameOwnerActive) {
		// Arrange:
		for (auto height : { Height(75), Height(122) }) {
			// - create a root namespace with a finite duration
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = CreateRootNotification(signer, NamespaceId(25));
			notification.Duration = ArtifactDuration(1100);

			// Act: renew it before its expiry
			RunRootTest(
					notification,
					ObserverTestContext(NotifyMode::Commit, height),
					SeedCacheWithRoot25TreeSigner(signer),
					[&signer](auto& namespaceCacheDelta) {
						// Assert: the root was renewed - [initial start, initial end + duration)
						AssertRootRenewed(namespaceCacheDelta, signer, Height(10), Height(1223));
					});
		}
	}

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_RootRenewalSameOwnerInactive) {
		// Arrange:
		for (auto height : { Height(123), Height(250) }) {
			// - create a root namespace with a finite duration
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = CreateRootNotification(signer, NamespaceId(25));
			notification.Duration = ArtifactDuration(1100);

			// Act: renew it after its expiry
			RunRootTest(
					notification,
					ObserverTestContext(NotifyMode::Commit, height),
					SeedCacheWithRoot25TreeSigner(signer),
					[height, &signer](auto& namespaceCacheDelta) {
						// Assert: the root was renewed - [block height, block height + duration)
						AssertRootRenewed(namespaceCacheDelta, signer, height, height + Height(1100));
					});
		}
	}

	namespace {
		void AssertRootOwnerChanged(
				cache::NamespaceCacheDelta& namespaceCacheDelta,
				const Key& signer,
				Height lifetimeStart,
				Height lifetimeEnd) {
			// Assert: the root was renewed
			EXPECT_EQ(1u, namespaceCacheDelta.activeSize());
			EXPECT_EQ(3u, namespaceCacheDelta.deepSize());
			ASSERT_TRUE(namespaceCacheDelta.contains(NamespaceId(25)));

			const auto& entry = namespaceCacheDelta.get(NamespaceId(25));
			EXPECT_EQ(Namespace_Base_Id, entry.ns().parentId());

			EXPECT_EQ(signer, entry.root().owner());
			EXPECT_EQ(lifetimeStart, entry.root().lifetime().Start);
			EXPECT_EQ(lifetimeEnd, entry.root().lifetime().End);
			EXPECT_TRUE(entry.root().empty());
		}
	}

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_RootRenewalChangeOwnerActive) {
		// Arrange:
		for (auto height : { Height(75), Height(122) }) {
			// - create a root namespace with a finite duration
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = CreateRootNotification(signer, NamespaceId(25));
			notification.Duration = ArtifactDuration(1100);

			// Act: change owner before its expiry (this is prevented by a validator)
			RunRootTest(
					notification,
					ObserverTestContext(NotifyMode::Commit, height),
					SeedCacheWithRoot25TreeSigner(test::CreateRandomOwner()),
					[height, &signer](auto& namespaceCacheDelta) {
						// Assert: the root was renewed - [block height, block height + duration)
						AssertRootOwnerChanged(namespaceCacheDelta, signer, height, height + Height(1100));
					});
		}
	}

	TEST(TEST_CLASS, ObserverAddsNamespaceOnCommit_RootRenewalChangeOwnerInactive) {
		// Arrange:
		for (auto height : { Height(123), Height(250) }) {
			// - create a root namespace with a finite duration
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = CreateRootNotification(signer, NamespaceId(25));
			notification.Duration = ArtifactDuration(1100);

			// Act: change owner after its expiry
			RunRootTest(
					notification,
					ObserverTestContext(NotifyMode::Commit, height),
					SeedCacheWithRoot25TreeSigner(test::CreateRandomOwner()),
					[height, &signer](auto& namespaceCacheDelta) {
						// Assert: the root was renewed - [block height, block height + duration)
						AssertRootOwnerChanged(namespaceCacheDelta, signer, height, height + Height(1100));
					});
		}
	}

	// endregion

	// region rollback

	TEST(TEST_CLASS, ObserverRemovesNamespaceOnRollback_Root) {
		// Arrange: create a root namespace for removal
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateRootNotification(signer, NamespaceId(25));

		// Act: remove it
		RunRootTest(
				notification,
				ObserverTestContext(NotifyMode::Rollback),
				[&signer](auto& namespaceCacheDelta) {
					// Arrange: create a cache with { 25, 26 }
					namespaceCacheDelta.insert(state::RootNamespace(NamespaceId(25), signer, test::CreateLifetime(10, 20)));
					namespaceCacheDelta.insert(state::RootNamespace(NamespaceId(26), signer, test::CreateLifetime(10, 20)));

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
