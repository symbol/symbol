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
		model::NotificationContext CreateNotificationContext(Height height) {
			return model::NotificationContext(height, model::ResolverContext());
		}

		void AssertPruningPredicate(Height height, NotifyMode mode, size_t pruneInterval, bool expectedResult) {
			// Arrange:
			auto cache = test::CreateEmptyCatapultCache();
			auto cacheDelta = cache.createDelta();
			ObserverContext context(CreateNotificationContext(height), ObserverState(cacheDelta), mode);

			// Act:
			auto result = ShouldPrune(context, pruneInterval);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "height " << height << ", pruneInterval " << pruneInterval;
		}
	}

	TEST(TEST_CLASS, ShouldPruneReturnsTrueWhenConditionsAreMet) {
		AssertPruningPredicate(Height(1), NotifyMode::Commit, 1, true);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 1, true);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 2, true);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 4, true);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 5, true);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 10, true);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 20, true);
	}

	TEST(TEST_CLASS, ShouldPruneReturnsFalseForModeRollback) {
		AssertPruningPredicate(Height(10), NotifyMode::Rollback, 1, false);
		AssertPruningPredicate(Height(10), NotifyMode::Rollback, 2, false);
		AssertPruningPredicate(Height(20), NotifyMode::Rollback, 5, false);
		AssertPruningPredicate(Height(50), NotifyMode::Rollback, 25, false);
	}

	TEST(TEST_CLASS, ShouldPruneReturnsFalseWhenHeightIsNotDivisibleByPruneInterval) {
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 3, false);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 6, false);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 19, false);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 21, false);
		AssertPruningPredicate(Height(20), NotifyMode::Commit, 123, false);
	}

	// endregion

	// region ShouldLink

	namespace {
		enum class FooAction { Link, Unlink };

		void AssertShouldLinkPredicate(FooAction action, NotifyMode notifyMode, bool expectedResult) {
			// Act:
			auto result = ShouldLink(action, notifyMode);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "action " << utils::to_underlying_type(action) << ", notifyMode " << notifyMode;
		}
	}

	TEST(TEST_CLASS, ShouldLinkReturnsCorrectValue) {
		AssertShouldLinkPredicate(FooAction::Link, NotifyMode::Commit, true);
		AssertShouldLinkPredicate(FooAction::Link, NotifyMode::Rollback, false);

		AssertShouldLinkPredicate(FooAction::Unlink, NotifyMode::Commit, false);
		AssertShouldLinkPredicate(FooAction::Unlink, NotifyMode::Rollback, true);
	}

	// endregion

	namespace {
		// region PrunableCache

		// 1. emulate a delta cache that supports (block and time) pruning and (block) touching
		// 2. derive from test::SimpleCacheDelta to ensure all required functions are provided (but unused)
		// 3. support block and time pruning and block touching to minimize cruft
		class PrunableCacheDelta : public test::SimpleCacheDelta {
		public:
			PrunableCacheDelta(std::vector<Height>& pruneHeights, std::vector<Timestamp>& pruneTimes, std::vector<Height>& touchHeights)
					: test::SimpleCacheDelta(test::SimpleCacheViewMode::Basic, test::SimpleCacheState())
					, m_pruneHeights(pruneHeights)
					, m_pruneTimes(pruneTimes)
					, m_touchHeights(touchHeights)
			{}

		public:
			void prune(Height height) {
				m_pruneHeights.push_back(height);
			}

			void prune(Timestamp time) {
				m_pruneTimes.push_back(time);
			}

			std::vector<Timestamp> touch(Height height) {
				m_touchHeights.push_back(height);

				// emulate ids (Timestamp) grouped by height but return them in reverse order to test deterministic ordering
				std::vector<Timestamp> timestamps;
				for (auto i = 10u; i < height.unwrap(); i += 10)
					timestamps.push_back(Timestamp(i));

				std::reverse(timestamps.begin(), timestamps.end());
				return timestamps;
			}

		private:
			std::vector<Height>& m_pruneHeights;
			std::vector<Timestamp>& m_pruneTimes;
			std::vector<Height>& m_touchHeights;
		};

		// 1. emulate a basic cache that supports pruning and touching
		// 2. derive from test::BasicSimpleCache to ensure all required functions are provided (but unused)
		// 3. createDetachedDelta needs explicit implementation because return type needs to match that of createDelta
		class BasicPrunableCache : public test::BasicSimpleCache {
		public:
			using CacheDeltaType = PrunableCacheDelta;

		public:
			BasicPrunableCache(std::vector<Height>& pruneHeights, std::vector<Timestamp>& pruneTimes, std::vector<Height>& touchHeights)
					: test::BasicSimpleCache(nullptr)
					, m_pruneHeights(pruneHeights)
					, m_pruneTimes(pruneTimes)
					, m_touchHeights(touchHeights)
			{}

		public:
			CacheDeltaType createDelta() {
				return PrunableCacheDelta(m_pruneHeights, m_pruneTimes, m_touchHeights);
			}

			CacheDeltaType createDetachedDelta() const {
				CATAPULT_THROW_RUNTIME_ERROR("createDetachedDelta - not supported");
			}

		private:
			std::vector<Height>& m_pruneHeights;
			std::vector<Timestamp>& m_pruneTimes;
			std::vector<Height>& m_touchHeights;
		};

		// 1. wrap BasicPrunableCache in cache::SynchronizedCache so it can be registered with CatapultCache
		// 2. expose collected prune heights and times
		class PrunableCache : public cache::SynchronizedCache<BasicPrunableCache> {
		public:
			static constexpr auto Id = 0;
			static constexpr auto Name = "PrunableCache";

		public:
			PrunableCache() : SynchronizedCache(BasicPrunableCache(m_pruneHeights, m_pruneTimes, m_touchHeights))
			{}

		public:
			std::vector<Height> pruneHeights() const {
				return m_pruneHeights;
			}

			std::vector<Timestamp> pruneTimes() const {
				return m_pruneTimes;
			}

			std::vector<Height> touchHeights() const {
				return m_touchHeights;
			}

		private:
			std::vector<Height> m_pruneHeights;
			std::vector<Timestamp> m_pruneTimes;
			std::vector<Height> m_touchHeights;
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

		void NotifyBlock(const PruningObserver& observer, ObserverContext& context, Timestamp timestamp) {
			observer.notify(model::BlockNotification(Address(), Address(), timestamp, Difficulty(), BlockFeeMultiplier()), context);
		}

		void NotifyBlock(const PruningObserver& observer, ObserverContext& context) {
			NotifyBlock(observer, context, Timestamp());
		}

		void AssertNoPruning(const PruningObserver& observer, NotifyMode mode, Height height) {
			// Arrange:
			auto cache = CreateSimpleCatapultCache();
			auto cacheDelta = cache.createDelta();
			ObserverContext context(CreateNotificationContext(height), ObserverState(cacheDelta), mode);

			// Act:
			NotifyBlock(observer, context);
			const auto& subCache = cache.sub<PrunableCache>();

			// Assert:
			auto message = CreateMessage(mode, height);
			EXPECT_TRUE(subCache.pruneHeights().empty()) << message;
			EXPECT_TRUE(subCache.pruneTimes().empty()) << message;
			EXPECT_TRUE(subCache.touchHeights().empty()) << message;
		}

		void AssertTimePruning(const PruningObserver& observer, NotifyMode mode, Height height, Timestamp timestamp) {
			// Arrange:
			auto cache = CreateSimpleCatapultCache();
			auto cacheDelta = cache.createDelta();
			ObserverContext context(CreateNotificationContext(height), ObserverState(cacheDelta), mode);

			// Act:
			NotifyBlock(observer, context, timestamp);
			const auto& subCache = cache.sub<PrunableCache>();

			// Assert:
			auto message = CreateMessage(mode, height);
			EXPECT_TRUE(subCache.pruneHeights().empty()) << message;
			EXPECT_EQ(std::vector<Timestamp>({ timestamp }), subCache.pruneTimes()) << message;
			EXPECT_TRUE(subCache.touchHeights().empty()) << message;
		}
	}

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

	TEST(TEST_CLASS, CacheTimePruningObserverPrunesWhenHeightIsDivisibleByPruneInterval) {
		// Arrange:
		auto pObserver = CreateCacheTimePruningObserver<PrunableCache>("Foo", 10);

		// Act + Assert:
		auto mode = NotifyMode::Commit;
		AssertTimePruning(*pObserver, mode, Height(10), Timestamp(97));
		AssertTimePruning(*pObserver, mode, Height(20), Timestamp(20));
		AssertTimePruning(*pObserver, mode, Height(30), Timestamp(7));
	}

	// endregion

	// region CacheBlockTouchObserver

	namespace {
		constexpr auto Receipt_Type_Marker = static_cast<model::ReceiptType>(0xA5A5);

		void AssertTouching(
				const PruningObserver& observer,
				NotifyMode mode,
				Height observerHeight,
				Height expectedTouchHeight,
				const std::vector<Timestamp::ValueType>& expectedExpiryIds = {}) {
			// Arrange:
			auto cache = CreateSimpleCatapultCache();
			auto cacheDelta = cache.createDelta();
			model::BlockStatementBuilder statementBuilder;
			ObserverContext context(CreateNotificationContext(observerHeight), ObserverState(cacheDelta, statementBuilder), mode);

			// Act:
			NotifyBlock(observer, context);
			const auto& subCache = cache.sub<PrunableCache>();

			// Assert:
			auto message = CreateMessage(mode, observerHeight);
			EXPECT_TRUE(subCache.pruneHeights().empty()) << message;
			EXPECT_TRUE(subCache.pruneTimes().empty()) << message;
			EXPECT_EQ(std::vector<Height>({ expectedTouchHeight }), subCache.touchHeights()) << message;

			// - check receipts
			auto pStatement = statementBuilder.build();
			if (expectedExpiryIds.empty()) {
				ASSERT_EQ(0u, pStatement->TransactionStatements.size());
				return;
			}

			ASSERT_EQ(1u, pStatement->TransactionStatements.size());
			const auto& receiptPair = *pStatement->TransactionStatements.find(model::ReceiptSource());
			ASSERT_EQ(expectedExpiryIds.size(), receiptPair.second.size());

			auto i = 0u;
			using ExpiryReceiptType = model::ArtifactExpiryReceipt<Timestamp>;
			for (auto id : expectedExpiryIds) {
				const auto& receipt = static_cast<const ExpiryReceiptType&>(receiptPair.second.receiptAt(i));
				ASSERT_EQ(sizeof(ExpiryReceiptType), receipt.Size) << i;
				EXPECT_EQ(1u, receipt.Version) << i;
				EXPECT_EQ(Receipt_Type_Marker, receipt.Type) << i;
				EXPECT_EQ(Timestamp(id), receipt.ArtifactId) << i;
				++i;
			}
		}
	}

	TEST(TEST_CLASS, CacheBlockTouchObserverIsCreatedWithCorrectName) {
		// Act:
		auto pObserver = CreateCacheBlockTouchObserver<PrunableCache>("Foo", Receipt_Type_Marker);

		// Assert:
		EXPECT_EQ("FooTouchObserver", pObserver->name());
	}

	TEST(TEST_CLASS, CacheBlockTouchObserverTouches_Commit) {
		// Arrange:
		auto pObserver = CreateCacheBlockTouchObserver<PrunableCache>("Foo", Receipt_Type_Marker);

		// Act + Assert:
		auto mode = NotifyMode::Commit;
		AssertTouching(*pObserver, mode, Height(1), Height(1), {});
		AssertTouching(*pObserver, mode, Height(11), Height(11), { 10 });
		AssertTouching(*pObserver, mode, Height(50), Height(50), { 10, 20, 30, 40 });
	}

	TEST(TEST_CLASS, CacheBlockTouchObserverTouches_Rollback) {
		// Arrange:
		auto pObserver = CreateCacheBlockTouchObserver<PrunableCache>("Foo", Receipt_Type_Marker);

		// Act + Assert:
		auto mode = NotifyMode::Rollback;
		AssertTouching(*pObserver, mode, Height(1), Height(1));
		AssertTouching(*pObserver, mode, Height(11), Height(11));
		AssertTouching(*pObserver, mode, Height(50), Height(50));
	}

	// endregion

	// region CacheBlockTouchObserver (nonzero grace period)

	TEST(TEST_CLASS, NonzeroGracePeriod_CacheBlockTouchObserverIsCreatedWithCorrectName) {
		// Act:
		auto pObserver = CreateCacheBlockTouchObserver<PrunableCache>("Foo", Receipt_Type_Marker, BlockDuration(100));

		// Assert:
		EXPECT_EQ("FooTouchObserver", pObserver->name());
	}

	TEST(TEST_CLASS, NonzeroGracePeriod_CacheBlockTouchObserverTouches_Commit) {
		// Arrange:
		auto pObserver = CreateCacheBlockTouchObserver<PrunableCache>("Foo", Receipt_Type_Marker, BlockDuration(10));

		// Act + Assert:
		auto mode = NotifyMode::Commit;
		AssertTouching(*pObserver, mode, Height(1), Height(11), { 10 });
		AssertTouching(*pObserver, mode, Height(11), Height(21), { 10, 20 });
		AssertTouching(*pObserver, mode, Height(43), Height(53), { 10, 20, 30, 40, 50 });
		AssertTouching(*pObserver, mode, Height(90), Height(100), { 10, 20, 30, 40, 50, 60, 70, 80, 90 });
	}

	TEST(TEST_CLASS, NonzeroGracePeriod_CacheBlockTouchObserverTouches_Rollback) {
		// Arrange:
		auto pObserver = CreateCacheBlockTouchObserver<PrunableCache>("Foo", Receipt_Type_Marker, BlockDuration(10));

		// Act + Assert:
		auto mode = NotifyMode::Rollback;
		AssertTouching(*pObserver, mode, Height(1), Height(11));
		AssertTouching(*pObserver, mode, Height(11), Height(21));
		AssertTouching(*pObserver, mode, Height(43), Height(53));
		AssertTouching(*pObserver, mode, Height(90), Height(100));
	}

	// endregion
}}
