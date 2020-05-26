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
#include "catapult/cache_core/BlockStatisticCache.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace observers {

#define TEST_CLASS BlockStatisticObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(BlockStatistic, 0, BlockFeeMultiplier())

	namespace {
		constexpr size_t Current_Height = 10;
		constexpr auto Base_Difficulty = Difficulty().unwrap();

		void SeedCache(cache::BlockStatisticCacheDelta& cache) {
			for (auto i = 1u; i <= Current_Height; ++i) {
				auto feeMultiplier = BlockFeeMultiplier(100 + i);
				auto statistic = state::BlockStatistic(Height(i), Timestamp(i), Difficulty(Base_Difficulty + i), feeMultiplier);
				cache.insert(statistic);
			}
		}

		state::BlockStatistic CreateStatistic() {
			return state::BlockStatistic(
					Height(Current_Height + 1),
					Timestamp(123),
					Difficulty(Base_Difficulty + 345),
					BlockFeeMultiplier(456));
		}

		BlockNotification CreateBlockNotification(const state::BlockStatistic& statistic) {
			return BlockNotification(Address(), Address(), statistic.Timestamp, statistic.Difficulty, statistic.FeeMultiplier);
		}
	}

	TEST(TEST_CLASS, ObserverInsertsStatisticIntoCacheInModeCommit) {
		// Arrange:
		test::ObserverTestContext context(NotifyMode::Commit, Height(Current_Height + 1));
		auto pObserver = CreateBlockStatisticObserver(Current_Height + 2, BlockFeeMultiplier(100));

		auto statistic = CreateStatistic();
		auto notification = CreateBlockNotification(statistic);
		auto& cache = context.observerContext().Cache.sub<cache::BlockStatisticCache>();
		SeedCache(cache);

		// Sanity:
		EXPECT_EQ(Current_Height, cache.size());
		EXPECT_FALSE(cache.contains(statistic));

		// Act:
		ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(Current_Height + 1, cache.size());
		EXPECT_TRUE(cache.contains(statistic));

		// - find and check the last statistic
		state::BlockStatistic lastStatistic;
		auto pIterableView = cache.tryMakeIterableView();
		for (const auto& value : *pIterableView)
			lastStatistic = value;

		EXPECT_EQ(Height(Current_Height + 1), lastStatistic.Height);
		EXPECT_EQ(Timestamp(123), lastStatistic.Timestamp);
		EXPECT_EQ(Difficulty(Base_Difficulty + 345), lastStatistic.Difficulty);
		EXPECT_EQ(BlockFeeMultiplier(456), lastStatistic.FeeMultiplier);

		// - check that state was updated, multipliers = (100, 101, 102, 103, 104, 105, 106 ...)
		EXPECT_EQ(BlockFeeMultiplier(106), context.state().DynamicFeeMultiplier);
	}

	TEST(TEST_CLASS, ObserverRemovesStatisticFromCacheInModeRollback) {
		// Arrange:
		test::ObserverTestContext context(NotifyMode::Rollback, Height(Current_Height + 1));
		auto pObserver = CreateBlockStatisticObserver(Current_Height + 2, BlockFeeMultiplier(100));

		auto statistic = CreateStatistic();
		auto notification = CreateBlockNotification(statistic);
		auto& cache = context.observerContext().Cache.sub<cache::BlockStatisticCache>();
		SeedCache(cache);
		cache.insert(statistic);

		// Sanity:
		EXPECT_EQ(Current_Height + 1, cache.size());
		EXPECT_TRUE(cache.contains(statistic));

		// Act:
		ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(Current_Height, cache.size());
		EXPECT_FALSE(cache.contains(statistic));

		// - check that state was updated, multipliers = (100, 100, 101, 102, 103, 104, 105 ...)
		EXPECT_EQ(BlockFeeMultiplier(105), context.state().DynamicFeeMultiplier);
	}
}}
