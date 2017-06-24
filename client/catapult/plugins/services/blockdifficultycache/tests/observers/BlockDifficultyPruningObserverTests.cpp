#include "src/observers/Observers.h"
#include "src/cache/BlockDifficultyCache.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace observers {

	DEFINE_COMMON_OBSERVER_TESTS(BlockDifficultyPruning, 360)

	namespace {
		model::BlockChainConfiguration CreateConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.MaxRollbackBlocks = 100;
			config.MaxDifficultyBlocks = 500;
			config.BlockPruneInterval = 300;
			return config;
		}

		const uint64_t Difficulty_History_Size = CalculateDifficultyHistorySize(CreateConfiguration());
		const uint64_t Block_Prune_Interval = CreateConfiguration().BlockPruneInterval;

		void PrepareCache(cache::CatapultCacheDelta& cache, Height height) {
			// Arrange: fill cache with block difficulties for heights that are from 1 till height
			auto& difficultyCache = cache.sub<cache::BlockDifficultyCache>();
			for (auto i = 1u; i <= height.unwrap(); ++i) {
				auto info = state::BlockDifficultyInfo(Height(i));
				difficultyCache.insert(info);
			}
		}

		struct TestContext {
			explicit TestContext(Height height, Timestamp timestamp, NotifyMode mode)
					: ObserverContext(mode, height, CreateConfiguration())
					, pObserver(CreateBlockDifficultyPruningObserver(Block_Prune_Interval))
					, Notification(test::GenerateRandomData<Key_Size>(), timestamp, Difficulty()) {
				PrepareCache(ObserverContext.cache(), height);
			}

			const auto& blockDifficultyCache() {
				return ObserverContext.cache().sub<cache::BlockDifficultyCache>();
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

			test::ObserverTestContext ObserverContext;
			NotificationObserverPointerT<model::BlockNotification> pObserver;
			BlockNotification Notification;
		};

		void AssertDifficultyPruning(Height height, uint64_t expectedSize) {
			// Arrange:
			TestContext context(height, Timestamp(), NotifyMode::Commit);
			const auto& cache = context.blockDifficultyCache();
			const auto& observer = context.pruningObserver();

			// Sanity:
			EXPECT_EQ(height.unwrap(), cache.size());

			// Act:
			test::ObserveNotification(observer, context.Notification, context.context());
			context.commit();

			// Assert:
			EXPECT_EQ(expectedSize, cache.size());
			for (auto i = 0u; i < expectedSize; ++i) {
				auto info = state::BlockDifficultyInfo(Height(height.unwrap() - i));
				EXPECT_TRUE(cache.contains(info));
			}
		}

		void AssertNoPruning(Height height, NotifyMode mode) {
			// Arrange:
			TestContext context(height, Timestamp(), mode);
			const auto& cache = context.blockDifficultyCache();
			const auto& observer = context.pruningObserver();

			// Act:
			test::ObserveNotification(observer, context.Notification, context.context());
			context.commit();

			// Assert:
			EXPECT_EQ(height.unwrap(), cache.size());
		}
	}

	TEST(BlockDifficultyPruningObserverTests, PruningObserverPrunesDifficultiesInModeCommitAtExpectedHeights) {
		// Assert: note that config.PruneInterval < Difficulty_History_Size < 2 * config.PruneInterval
		AssertDifficultyPruning(Height(1), 1);
		AssertDifficultyPruning(Height(Block_Prune_Interval + 1), Block_Prune_Interval + 1);
		AssertDifficultyPruning(Height(2 * Block_Prune_Interval + 1), Difficulty_History_Size + 1);
		AssertDifficultyPruning(Height(10 * Block_Prune_Interval + 1), Difficulty_History_Size + 1);
	}

	TEST(BlockDifficultyPruningObserverTests, PruningObserverDoesNotPruneInModeRollback) {
		// Assert:
		auto mode = NotifyMode::Rollback;
		AssertNoPruning(Height(1), mode);
		AssertNoPruning(Height(Block_Prune_Interval + 1), mode);
		AssertNoPruning(Height(2 * Block_Prune_Interval + 1), mode);
		AssertNoPruning(Height(10 * Block_Prune_Interval + 1), mode);
	}

	TEST(BlockDifficultyPruningObserverTests, PruningObserverDoesNotPruneAtNonPruneHeights) {
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
