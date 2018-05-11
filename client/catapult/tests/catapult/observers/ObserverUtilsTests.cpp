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

#include "catapult/observers/ObserverUtils.h"
#include "catapult/cache/CatapultCacheBuilder.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS ObserverUtilsTests

	// region ShouldPrune

	namespace {
		void AssertPruningPredicate(Height height, NotifyMode mode, size_t pruneInterval, bool expectedResult) {
			// Arrange:
			auto cache = test::CreateEmptyCatapultCache();
			auto cacheDelta = cache.createDelta();
			state::CatapultState state;
			ObserverContext context(cacheDelta, state, height, mode);

			// Act:
			auto result = ShouldPrune(context, pruneInterval);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "height " << height << ", pruneInterval " << pruneInterval;
		}
	}

	TEST(TEST_CLASS, ShouldPruneReturnsTrueIfConditionsAreMet) {
		// Assert:
		AssertPruningPredicate(Height(1), NotifyMode::Commit, 1, true);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 1, true);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 2, true);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 4, true);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 5, true);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 10, true);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 20, true);
	}

	TEST(TEST_CLASS, ShouldPruneReturnsFalseForModeRollback) {
		// Assert:
		AssertPruningPredicate(Height(10), NotifyMode::Rollback, 1, false);
		AssertPruningPredicate(Height(10), NotifyMode::Rollback, 2, false);
		AssertPruningPredicate(Height(20), NotifyMode::Rollback, 5, false);
		AssertPruningPredicate(Height(50), NotifyMode::Rollback, 25, false);
	}

	TEST(TEST_CLASS, ShouldPruneReturnsFalseIfHeightIsNotDivisibleByPruneInterval) {
		// Assert:
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 3, false);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 6, false);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 19, false);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 21, false);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 123, false);
	}

	// endregion

	namespace {
		// region PrunableCache

		// 1. emulate a delta cache that supports (block and time) pruning
		// 2. derive from test::SimpleCacheDelta to ensure all required functions are provided (but unused)
		// 3. support both block and time pruning to minimize cruft
		class PrunableCacheDelta : public test::SimpleCacheDelta {
		public:
			PrunableCacheDelta(std::vector<Height>& pruneHeights, std::vector<Timestamp>& pruneTimes)
					: test::SimpleCacheDelta(0)
					, m_pruneHeights(pruneHeights)
					, m_pruneTimes(pruneTimes)
			{}

		public:
			void prune(Height height) {
				m_pruneHeights.push_back(height);
			}

			void prune(Timestamp time) {
				m_pruneTimes.push_back(time);
			}

		private:
			std::vector<Height>& m_pruneHeights;
			std::vector<Timestamp>& m_pruneTimes;
		};

		// 1. emulate a basic cache that supports pruning
		// 2. derive from test::BasicSimpleCache to ensure all required functions are provided (but unused)
		// 3. createDetachedDelta needs explicit implementation because return type needs to match that of createDelta
		class BasicPrunableCache : public test::BasicSimpleCache {
		public:
			using CacheDeltaType = PrunableCacheDelta;

		public:
			BasicPrunableCache(std::vector<Height>& pruneHeights, std::vector<Timestamp>& pruneTimes)
					: test::BasicSimpleCache(nullptr)
					, m_pruneHeights(pruneHeights)
					, m_pruneTimes(pruneTimes)
			{}

		public:
			CacheDeltaType createDelta() {
				return PrunableCacheDelta(m_pruneHeights, m_pruneTimes);
			}

			CacheDeltaType createDetachedDelta() const {
#ifndef _MSC_VER
				CATAPULT_THROW_RUNTIME_ERROR("createDetachedDelta - not supported");
#else
				// return something with invalid references to quiet unreachable code warning
				auto pruneHeights = std::vector<Height>();
				auto pruneTimes = std::vector<Timestamp>();
				return PrunableCacheDelta(pruneHeights, pruneTimes);
#endif
			}

		private:
			std::vector<Height>& m_pruneHeights;
			std::vector<Timestamp>& m_pruneTimes;
		};

		// 1. wrap BasicPrunableCache in cache::SynchronizedCache so it can be registered with CatapultCache
		// 2. expose collected prune heights and times
		class PrunableCache : public cache::SynchronizedCache<BasicPrunableCache> {
		public:
			static constexpr auto Id = 0;
			static constexpr auto Name = "PrunableCache";

		public:
			PrunableCache() : SynchronizedCache(BasicPrunableCache(m_pruneHeights, m_pruneTimes))
			{}

		public:
			std::vector<Height> pruneHeights() const {
				return m_pruneHeights;
			}

			std::vector<Timestamp> pruneTimes() const {
				return m_pruneTimes;
			}

		private:
			std::vector<Height> m_pruneHeights;
			std::vector<Timestamp> m_pruneTimes;
		};

		// endregion

		cache::CatapultCache CreateSimpleCatapultCache() {
			cache::CatapultCacheBuilder builder;
			builder.add<test::SimpleCacheStorageTraits>(std::make_unique<PrunableCache>());
			return builder.build();
		}

		std::string CreateMessage(NotifyMode mode, Height height) {
			std::ostringstream out;
			out << "height " << height << ", mode " << mode;
			return out.str();
		}

		using PruningObserver = NotificationObserverT<model::BlockNotification>;

		void AssertNoPruning(const PruningObserver& observer, NotifyMode mode, Height height) {
			// Arrange:
			auto cache = CreateSimpleCatapultCache();
			auto cacheDelta = cache.createDelta();
			state::CatapultState state;
			ObserverContext context(cacheDelta, state, height, mode);

			// Act:
			observer.notify(model::BlockNotification(Key(), Timestamp(), Difficulty()), context);
			const auto& subCache = cache.sub<PrunableCache>();

			// Assert:
			auto message = CreateMessage(mode, height);
			EXPECT_TRUE(subCache.pruneHeights().empty()) << message;
			EXPECT_TRUE(subCache.pruneTimes().empty()) << message;
		}

		void AssertBlockPruning(const PruningObserver& observer, NotifyMode mode, Height height, Height expectedPruneHeight) {
			// Arrange:
			auto cache = CreateSimpleCatapultCache();
			auto cacheDelta = cache.createDelta();
			state::CatapultState state;
			ObserverContext context(cacheDelta, state, height, mode);

			// Act:
			observer.notify(model::BlockNotification(Key(), Timestamp(), Difficulty()), context);
			const auto& subCache = cache.sub<PrunableCache>();

			// Assert:
			auto message = CreateMessage(mode, height);
			EXPECT_EQ(std::vector<Height>({ expectedPruneHeight }), subCache.pruneHeights()) << message;
			EXPECT_TRUE(subCache.pruneTimes().empty()) << message;
		}

		void AssertTimePruning(const PruningObserver& observer, NotifyMode mode, Height height, Timestamp timestamp) {
			// Arrange:
			auto cache = CreateSimpleCatapultCache();
			auto cacheDelta = cache.createDelta();
			state::CatapultState state;
			ObserverContext context(cacheDelta, state, height, mode);

			// Act:
			observer.notify(model::BlockNotification(Key(), timestamp, Difficulty()), context);
			const auto& subCache = cache.sub<PrunableCache>();

			// Assert:
			auto message = CreateMessage(mode, height);
			EXPECT_TRUE(subCache.pruneHeights().empty()) << message;
			EXPECT_EQ(std::vector<Timestamp>({ timestamp }), subCache.pruneTimes()) << message;
		}
	}

	// region CacheBlockPruningObserver

	TEST(TEST_CLASS, CacheBlockPruningObserverIsCreatedWithCorrectName) {
		// Act:
		auto pObserver = CreateCacheBlockPruningObserver<PrunableCache>("Foo", 10, BlockDuration(7));

		// Assert:
		EXPECT_EQ("FooPruningObserver", pObserver->name());
	}

	TEST(TEST_CLASS, CacheBlockPruningObserverSkipsPruningWhenModeIsRollback) {
		// Arrange:
		auto pObserver = CreateCacheBlockPruningObserver<PrunableCache>("Foo", 10, BlockDuration(7));

		// Act + Assert:
		auto mode = NotifyMode::Rollback;
		AssertNoPruning(*pObserver, mode, Height(1));
		AssertNoPruning(*pObserver, mode, Height(7));
		AssertNoPruning(*pObserver, mode, Height(10));
		AssertNoPruning(*pObserver, mode, Height(19));
		AssertNoPruning(*pObserver, mode, Height(20));
		AssertNoPruning(*pObserver, mode, Height(21));
		AssertNoPruning(*pObserver, mode, Height(30));
	}

	TEST(TEST_CLASS, CacheBlockPruningObserverSkipsPruningWhenHeightIsNotDivisibleByPruneInterval) {
		// Arrange:
		auto pObserver = CreateCacheBlockPruningObserver<PrunableCache>("Foo", 10, BlockDuration(7));

		// Act + Assert:
		auto mode = NotifyMode::Commit;
		AssertNoPruning(*pObserver, mode, Height(11));
		AssertNoPruning(*pObserver, mode, Height(19));
		AssertNoPruning(*pObserver, mode, Height(21));
		AssertNoPruning(*pObserver, mode, Height(29));
	}

	TEST(TEST_CLASS, CacheBlockPruningObserverSkipsPruningWhenHeightIsNotGreaterThanGracePeriod) {
		// Arrange:
		auto pObserver = CreateCacheBlockPruningObserver<PrunableCache>("Foo", 1, BlockDuration(7));

		// Act + Assert:
		auto mode = NotifyMode::Commit;
		AssertNoPruning(*pObserver, mode, Height(1));
		AssertNoPruning(*pObserver, mode, Height(4));
		AssertNoPruning(*pObserver, mode, Height(7));
	}

	TEST(TEST_CLASS, CacheBlockPruningObserverTriggersPruningWhenHeightIsGreaterThanGracePeriod) {
		// Arrange:
		auto pObserver = CreateCacheBlockPruningObserver<PrunableCache>("Foo", 1, BlockDuration(7));

		// Act + Assert:
		auto mode = NotifyMode::Commit;
		AssertBlockPruning(*pObserver, mode, Height(8), Height(1));
		AssertBlockPruning(*pObserver, mode, Height(9), Height(2));
		AssertBlockPruning(*pObserver, mode, Height(10), Height(3));
	}

	TEST(TEST_CLASS, CacheBlockPruningObserverTriggersPruningWhenHeightIsGreaterThanGracePeriodAndDivisibleByPruneInterval) {
		// Arrange:
		auto pObserver = CreateCacheBlockPruningObserver<PrunableCache>("Foo", 10, BlockDuration(7));

		// Act + Assert:
		auto mode = NotifyMode::Commit;
		AssertBlockPruning(*pObserver, mode, Height(10), Height(3));
		AssertBlockPruning(*pObserver, mode, Height(20), Height(13));
		AssertBlockPruning(*pObserver, mode, Height(30), Height(23));
	}

	// endregion

	// region CacheTimePruningObserver

	TEST(TEST_CLASS, CacheTimePruningObserverIsCreatedWithCorrectName) {
		// Act:
		auto pObserver = CreateCacheTimePruningObserver<PrunableCache>("Foo", 10);

		// Assert:
		EXPECT_EQ("FooPruningObserver", pObserver->name());
	}

	TEST(TEST_CLASS, CacheTimePruningObserverSkipsPruningWhenModeIsRollback) {
		// Arrange:
		auto pObserver = CreateCacheTimePruningObserver<PrunableCache>("Foo", 10);

		// Act + Assert:
		auto mode = NotifyMode::Rollback;
		AssertNoPruning(*pObserver, mode, Height(1));
		AssertNoPruning(*pObserver, mode, Height(7));
		AssertNoPruning(*pObserver, mode, Height(10));
		AssertNoPruning(*pObserver, mode, Height(19));
		AssertNoPruning(*pObserver, mode, Height(20));
		AssertNoPruning(*pObserver, mode, Height(21));
		AssertNoPruning(*pObserver, mode, Height(30));
	}

	TEST(TEST_CLASS, CacheTimePruningObserverSkipsPruningWhenHeightIsNotDivisibleByPruneInterval) {
		// Arrange:
		auto pObserver = CreateCacheTimePruningObserver<PrunableCache>("Foo", 10);

		// Act + Assert:
		auto mode = NotifyMode::Commit;
		AssertNoPruning(*pObserver, mode, Height(11));
		AssertNoPruning(*pObserver, mode, Height(19));
		AssertNoPruning(*pObserver, mode, Height(21));
		AssertNoPruning(*pObserver, mode, Height(29));
	}

	TEST(TEST_CLASS, CacheTimePruningObserverTriggersPruningWhenHeightIsDivisibleByPruneInterval) {
		// Arrange:
		auto pObserver = CreateCacheTimePruningObserver<PrunableCache>("Foo", 10);

		// Act + Assert:
		auto mode = NotifyMode::Commit;
		AssertTimePruning(*pObserver, mode, Height(10), Timestamp(97));
		AssertTimePruning(*pObserver, mode, Height(20), Timestamp(20));
		AssertTimePruning(*pObserver, mode, Height(30), Timestamp(7));
	}

	// endregion
}}
