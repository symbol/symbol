#include "src/observers/Observers.h"
#include "src/cache/NamespaceCache.h"
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {
	using ObserverTestContext = test::ObserverTestContextT<test::NamespaceCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(NamespacePruning, ArtifactDuration(31), 100)

	namespace {
		constexpr size_t Num_Roots = 6;
		constexpr size_t Block_Prune_Interval = 100;
		constexpr ArtifactDuration Grace_Period_Duration(31);

		void PrepareCache(cache::CatapultCacheDelta& cache, const Key& rootOwner) {
			auto& delta = cache.sub<cache::NamespaceCache>();
			for (auto i = 0u; i < Num_Roots - 1; ++i)
				delta.insert(state::RootNamespace(NamespaceId(i + 1), rootOwner, test::CreateLifetime(100, 100 * (i + 2))));

			delta.insert(state::RootNamespace(NamespaceId(10), rootOwner, test::CreateLifetime(1, 2)));
		}

		struct TestContext {
			explicit TestContext(Height height, NotifyMode mode)
					: ObserverContext(mode, height, model::BlockChainConfiguration::Uninitialized())
					, pObserver(CreateNamespacePruningObserver(Grace_Period_Duration, Block_Prune_Interval))
					, Owner(test::CreateRandomOwner())
					, Notification(test::CreateBlockNotification()) {
				PrepareCache(ObserverContext.cache(), Owner);
			}

			const auto& namespaceCache() {
				return ObserverContext.cache().sub<cache::NamespaceCache>();
			}

			const auto& pruningObserver() {
				return *pObserver;
			}

			const auto& context() {
				return ObserverContext.observerContext();
			}

			void commit() {
				ObserverContext.commitCacheChanges();
			}

			ObserverTestContext ObserverContext;
			NotificationObserverPointerT<model::BlockNotification> pObserver;
			Key Owner;
			model::BlockNotification Notification;
		};

		void AssertNamespacePruning(Height height, const std::vector<uint64_t>& expectedIds) {
			// Arrange:
			TestContext context(height, NotifyMode::Commit);
			const auto& cache = context.namespaceCache();
			const auto& observer = context.pruningObserver();

			// Sanity:
			EXPECT_EQ(Num_Roots, cache.size());
			EXPECT_EQ(Num_Roots, cache.activeSize());
			EXPECT_EQ(Num_Roots, cache.deepSize());

			// Act:
			test::ObserveNotification(observer, context.Notification, context.context());
			context.commit();

			// Assert:
			EXPECT_EQ(expectedIds.size(), cache.size());
			for (auto id : expectedIds)
				EXPECT_TRUE(cache.contains(NamespaceId(id))) << "id " << id;
		}

		void AssertNoPruning(Height height, NotifyMode mode) {
			// Arrange:
			TestContext context(height, mode);
			const auto& cache = context.namespaceCache();
			const auto& observer = context.pruningObserver();

			// Act:
			test::ObserveNotification(observer, context.Notification, context.context());
			context.commit();

			// Assert:
			EXPECT_EQ(Num_Roots, cache.size());
			EXPECT_EQ(Num_Roots, cache.activeSize());
			EXPECT_EQ(Num_Roots, cache.deepSize());
		}
	}

	TEST(NamespacePruningObserverTests, PruningObserverPrunesNamespacesInModeCommitAtExpectedHeights) {
		// Assert: namespaces have the following life
		// - id: end height / end height + grace period duration / will be pruned at height
		// - 1: 200 / 231 / 301
		// - 2: 300 / 331 / 401
		// - 3: 400 / 431 / 501
		// - 4: 500 / 531 / 601
		// - 5: 600 / 631 / 701
		// - 10: 2  / 33  / 101
		AssertNamespacePruning(Height(1), { 1u, 2u, 3u, 4u, 5u, 10u });
		AssertNamespacePruning(Height(Block_Prune_Interval + 1), { 1u, 2u, 3u, 4u, 5u });
		AssertNamespacePruning(Height(2 * Block_Prune_Interval + 1), { 1u, 2u, 3u, 4u, 5u });
		AssertNamespacePruning(Height(3 * Block_Prune_Interval + 1), { 2u, 3u, 4u, 5u });
		AssertNamespacePruning(Height(4 * Block_Prune_Interval + 1), { 3u, 4u, 5u });
		AssertNamespacePruning(Height(5 * Block_Prune_Interval + 1), { 4u, 5u });
		AssertNamespacePruning(Height(6 * Block_Prune_Interval + 1), { 5u });
		AssertNamespacePruning(Height(7 * Block_Prune_Interval + 1), {});
		AssertNamespacePruning(Height(10 * Block_Prune_Interval + 1), {});
	}

	TEST(NamespacePruningObserverTests, PruningObserverDoesNotPruneInModeRollback) {
		// Assert:
		auto mode = NotifyMode::Rollback;
		AssertNoPruning(Height(1), mode);
		AssertNoPruning(Height(Block_Prune_Interval + 1), mode);
		AssertNoPruning(Height(2 * Block_Prune_Interval + 1), mode);
		AssertNoPruning(Height(10 * Block_Prune_Interval + 1), mode);
	}

	TEST(NamespacePruningObserverTests, PruningObserverDoesNotPruneAtNonPruneHeights) {
		// Assert:
		auto mode = NotifyMode::Commit;
		AssertNoPruning(Height(2), mode);
		AssertNoPruning(Height(Block_Prune_Interval), mode);
		AssertNoPruning(Height(Block_Prune_Interval + 2), mode);
		AssertNoPruning(Height(2 * Block_Prune_Interval), mode);
		AssertNoPruning(Height(2 * Block_Prune_Interval + 2), mode);
		AssertNoPruning(Height(10 * Block_Prune_Interval), mode);
		AssertNoPruning(Height(10 * Block_Prune_Interval + 2), mode);
	}
}}
