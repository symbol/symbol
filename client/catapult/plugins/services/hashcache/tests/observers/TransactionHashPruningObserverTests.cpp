#include "src/observers/Observers.h"
#include "src/cache/HashCache.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/HashCacheTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {
	using ObserverTestContext = test::ObserverTestContextT<test::HashCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(TransactionHashPruning, 360)

	namespace {
		constexpr uint64_t Time_Offset = 1000;
		constexpr int64_t Range = 5;

		model::BlockChainConfiguration CreateConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.MaxTransactionLifetime = utils::TimeSpan::FromHours(12);
			config.BlockPruneInterval = 300;
			return config;
		}

		const uint64_t Transaction_Cache_Duration_Ms = CalculateTransactionCacheDuration(CreateConfiguration()).millis();
		const uint64_t Block_Prune_Interval = CreateConfiguration().BlockPruneInterval;

		void PrepareCache(cache::CatapultCacheDelta& cache) {
			// Arrange: fill cache with timestamped hashes that are before and after Time_Offset
			auto& hashCache = cache.sub<cache::HashCache>();
			for (auto delta = -Range; delta <= Range; ++delta) {
				Timestamp timestamp(Time_Offset + static_cast<Timestamp::ValueType>(delta));
				auto timestampedHash = state::TimestampedHash(timestamp, Hash256{});
				hashCache.insert(timestampedHash);
			}
		}

		struct TestContext {
			explicit TestContext(Height height, Timestamp timestamp, NotifyMode mode)
					: ObserverContext(mode, height, CreateConfiguration())
					, pObserver(CreateTransactionHashPruningObserver(Block_Prune_Interval))
					, Notification(test::GenerateRandomData<Key_Size>(), timestamp, Difficulty()) {
				PrepareCache(ObserverContext.cache());
			}

			const auto& hashCache() {
				return ObserverContext.cache().sub<cache::HashCache>();
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
			model::BlockNotification Notification;
		};

		void AssertHashPruning(Height height, Timestamp pruneTime, uint64_t expectedSize) {
			// Arrange:
			TestContext context(height, pruneTime, NotifyMode::Commit);
			const auto& cache = context.hashCache();
			const auto& observer = context.pruningObserver();

			// Sanity:
			EXPECT_EQ(2 * Range + 1, cache.size());

			// Act:
			test::ObserveNotification(observer, context.Notification, context.context());
			context.commit();

			// Assert:
			EXPECT_EQ(expectedSize, cache.size());
			for (auto i = 0u; i < expectedSize; ++i) {
				auto timestampedHash = state::TimestampedHash(Timestamp(Time_Offset + Range - i), Hash256{});
				EXPECT_TRUE(cache.contains(timestampedHash));
			}
		}

		void AssertNoPruning(Height height, NotifyMode mode) {
			// Arrange:
			Timestamp timestamp(Transaction_Cache_Duration_Ms + Time_Offset);
			TestContext context(height, timestamp, mode);
			const auto& hashCache = context.hashCache();
			const auto& observer = context.pruningObserver();

			// Act:
			test::ObserveNotification(observer, context.Notification, context.context());
			context.commit();

			// Assert:
			EXPECT_EQ(2 * Range + 1, hashCache.size());
		}
	}

	TEST(TransactionHashPruningObserverTests, PruningObserverPrunesHashesInModeCommitAtExpectedHeights) {
		// Assert:
		auto baseTime = Transaction_Cache_Duration_Ms + Time_Offset;
		for (auto delta = -Range; delta <= Range; ++delta) {
			// the cache initially will have 2 * Range + 1 timestamped hashes.
			// for delta = -Range no hash is pruned, delta = -Range + 1 will prune one hash and so on
			auto expectedSize = static_cast<uint64_t>(Range + 1 - delta);
			auto pruneTime = Timestamp(baseTime + static_cast<Timestamp::ValueType>(delta));
			AssertHashPruning(Height(1), pruneTime, expectedSize);
			AssertHashPruning(Height(Block_Prune_Interval + 1), pruneTime, expectedSize);
			AssertHashPruning(Height(2 * Block_Prune_Interval + 1), pruneTime, expectedSize);
			AssertHashPruning(Height(10 * Block_Prune_Interval + 1), pruneTime, expectedSize);
		}
	}

	TEST(TransactionHashPruningObserverTests, PruningObserverDoesNotPruneInModeRollback) {
		// Assert:
		auto mode = NotifyMode::Rollback;
		AssertNoPruning(Height(1), mode);
		AssertNoPruning(Height(Block_Prune_Interval + 1), mode);
		AssertNoPruning(Height(2 * Block_Prune_Interval + 1), mode);
		AssertNoPruning(Height(10 * Block_Prune_Interval + 1), mode);
	}

	TEST(TransactionHashPruningObserverTests, PruningObserverDoesNotPruneAtNonPruneHeights) {
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
