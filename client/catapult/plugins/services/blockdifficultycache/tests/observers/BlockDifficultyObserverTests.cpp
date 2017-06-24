#include "src/observers/Observers.h"
#include "src/cache/BlockDifficultyCache.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace observers {

	DEFINE_COMMON_OBSERVER_TESTS(BlockDifficulty,)

	namespace {
		constexpr size_t Current_Height = 10;
		constexpr auto Base_Difficulty = Difficulty().unwrap();

		void SeedCache(cache::BlockDifficultyCacheDelta& cache) {
			for (auto i = 1u; i <= Current_Height; ++i) {
				auto info = state::BlockDifficultyInfo(Height(i), Timestamp(i), Difficulty(Base_Difficulty + i));
				cache.insert(info);
			}
		}

		BlockNotification CreateBlockNotification(const state::BlockDifficultyInfo& info) {
			return BlockNotification(test::GenerateRandomData<Key_Size>(), info.BlockTimestamp, info.BlockDifficulty);
		}
	}

	TEST(BlockDifficultyObserverTests, ObserverInsertsDifficultyInfoIntoCacheInModeCommit) {
		// Arrange:
		test::ObserverTestContext context(NotifyMode::Commit, Height(Current_Height + 1));
		auto pObserver = CreateBlockDifficultyObserver();
		auto info = state::BlockDifficultyInfo(Height(Current_Height + 1), Timestamp(123), Difficulty(Base_Difficulty + 345));
		auto notification = CreateBlockNotification(info);
		auto& cache = context.observerContext().Cache.sub<cache::BlockDifficultyCache>();
		SeedCache(cache);

		// Sanity:
		EXPECT_EQ(Current_Height, cache.size());
		EXPECT_FALSE(cache.contains(info));

		// Act:
		ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(Current_Height + 1, cache.size());
		EXPECT_TRUE(cache.contains(info));
	}

	TEST(BlockDifficultyObserverTests, ObserverRemovesDifficultyInfoFromCacheInModeRollback) {
		// Arrange:
		test::ObserverTestContext context(NotifyMode::Rollback, Height(Current_Height + 1));
		auto pObserver = CreateBlockDifficultyObserver();
		auto info = state::BlockDifficultyInfo(Height(Current_Height + 1), Timestamp(123), Difficulty(Base_Difficulty + 345));
		auto notification = CreateBlockNotification(info);
		auto& cache = context.observerContext().Cache.sub<cache::BlockDifficultyCache>();
		SeedCache(cache);
		cache.insert(info);

		// Sanity:
		EXPECT_EQ(Current_Height + 1, cache.size());
		EXPECT_TRUE(cache.contains(info));

		// Act:
		ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(Current_Height, cache.size());
		EXPECT_FALSE(cache.contains(info));
	}
}}
