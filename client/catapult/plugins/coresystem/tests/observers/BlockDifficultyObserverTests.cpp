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
#include "catapult/cache_core/BlockDifficultyCache.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace observers {

#define TEST_CLASS BlockDifficultyObserverTests

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
			return BlockNotification(test::GenerateRandomByteArray<Key>(), Key(), info.BlockTimestamp, info.BlockDifficulty);
		}
	}

	TEST(TEST_CLASS, ObserverInsertsDifficultyInfoIntoCacheInModeCommit) {
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

	TEST(TEST_CLASS, ObserverRemovesDifficultyInfoFromCacheInModeRollback) {
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
