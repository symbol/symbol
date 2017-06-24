#include "src/observers/Observers.h"
#include "src/cache/HashCache.h"
#include "catapult/model/EntityHasher.h"
#include "tests/test/HashCacheTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {
	using ObserverTestContext = test::ObserverTestContextT<test::HashCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(TransactionHash,)

	namespace {
		constexpr size_t Num_Hashes = 10;

		void SeedCache(cache::HashCacheDelta& cache) {
			for (auto i = 0u; i < Num_Hashes; ++i) {
				cache.insert(state::TimestampedHash(Timestamp(i), test::GenerateRandomData<Hash256_Size>()));
			}
		}

		state::TimestampedHash CreateTimestampedHash(Timestamp deadline, const Hash256& hash) {
			return state::TimestampedHash(deadline, hash);
		}

		model::TransactionNotification MakeNotification(Timestamp deadline, const Hash256& hash) {
			return model::TransactionNotification(Key(), hash, deadline);
		}
	}

	TEST(TransactionHashObserverTests, ObserverInsertsHashIntoCacheInModeCommit) {
		// Arrange:
		ObserverTestContext context(NotifyMode::Commit);
		auto pObserver = CreateTransactionHashObserver();

		auto deadline = test::GenerateRandomValue<Timestamp>();
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto timestampedHash = CreateTimestampedHash(deadline, hash);

		auto& cache = context.observerContext().Cache.sub<cache::HashCache>();
		SeedCache(cache);

		// Sanity:
		EXPECT_EQ(Num_Hashes, cache.size());
		EXPECT_FALSE(cache.contains(timestampedHash));

		// Act:
		pObserver->notify(MakeNotification(deadline, hash), context.observerContext());

		// Assert:
		EXPECT_EQ(Num_Hashes + 1, cache.size());
		EXPECT_TRUE(cache.contains(timestampedHash));
	}

	TEST(TransactionHashObserverTests, ObserverRemovesHashFromCacheInModeRollback) {
		// Arrange:
		ObserverTestContext context(NotifyMode::Rollback);
		auto pObserver = CreateTransactionHashObserver();

		auto deadline = test::GenerateRandomValue<Timestamp>();
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto timestampedHash = CreateTimestampedHash(deadline, hash);

		auto& cache = context.observerContext().Cache.sub<cache::HashCache>();
		SeedCache(cache);
		cache.insert(timestampedHash);

		// Sanity:
		EXPECT_EQ(Num_Hashes + 1, cache.size());
		EXPECT_TRUE(cache.contains(timestampedHash));

		// Act:
		pObserver->notify(MakeNotification(deadline, hash), context.observerContext());

		// Assert:
		EXPECT_EQ(Num_Hashes, cache.size());
		EXPECT_FALSE(cache.contains(timestampedHash));
	}
}}
