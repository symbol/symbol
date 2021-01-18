/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS ObserverUtilsTests

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
		// region TouchableCache

		// 1. emulate a delta cache that supports (block) touching
		// 2. derive from test::SimpleCacheDelta to ensure all required functions are provided (but unused)
		class TouchableCacheDelta : public test::SimpleCacheDelta {
		public:
			explicit TouchableCacheDelta(std::vector<Height>& touchHeights)
					: test::SimpleCacheDelta(test::SimpleCacheViewMode::Basic, test::SimpleCacheState())
					, m_touchHeights(touchHeights)
			{}

		public:
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
			std::vector<Height>& m_touchHeights;
		};

		// 1. emulate a basic cache that supports (block) touching
		// 2. derive from test::BasicSimpleCache to ensure all required functions are provided (but unused)
		// 3. createDetachedDelta needs explicit implementation because return type needs to match that of createDelta
		class BasicTouchableCache : public test::BasicSimpleCache {
		public:
			using CacheDeltaType = TouchableCacheDelta;

		public:
			explicit BasicTouchableCache(std::vector<Height>& touchHeights)
					: test::BasicSimpleCache(nullptr)
					, m_touchHeights(touchHeights)
			{}

		public:
			CacheDeltaType createDelta() {
				return TouchableCacheDelta(m_touchHeights);
			}

			CacheDeltaType createDetachedDelta() const {
				CATAPULT_THROW_RUNTIME_ERROR("createDetachedDelta - not supported");
			}

		private:
			std::vector<Height>& m_touchHeights;
		};

		// 1. wrap BasicTouchableCache in cache::SynchronizedCache so it can be registered with CatapultCache
		// 2. expose collected touch heights
		class TouchableCache : public cache::SynchronizedCache<BasicTouchableCache> {
		public:
			static constexpr auto Id = 0;
			static constexpr auto Name = "TouchableCache";

		public:
			TouchableCache() : SynchronizedCache(BasicTouchableCache(m_touchHeights))
			{}

		public:
			std::vector<Height> touchHeights() const {
				return m_touchHeights;
			}

		private:
			std::vector<Height> m_touchHeights;
		};

		// endregion

		cache::CatapultCache CreateSimpleCatapultCache() {
			cache::CatapultCacheBuilder builder;
			builder.add<test::SimpleCacheStorageTraits>(std::make_unique<TouchableCache>());
			return builder.build();
		}

		std::string CreateMessage(NotifyMode mode, Height height) {
			std::ostringstream out;
			out << "height " << height << ", mode " << mode;
			return out.str();
		}

		using TouchingObserver = NotificationObserverT<model::BlockNotification>;

		model::NotificationContext CreateNotificationContext(Height height) {
			return model::NotificationContext(height, model::ResolverContext());
		}

		void NotifyBlock(const TouchingObserver& observer, ObserverContext& context) {
			observer.notify(test::CreateBlockNotification(), context);
		}
	}

	// region CacheBlockTouchObserver

	namespace {
		constexpr auto Receipt_Type_Marker = static_cast<model::ReceiptType>(0xA5A5);

		void AssertTouching(
				const TouchingObserver& observer,
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
			const auto& subCache = cache.sub<TouchableCache>();

			// Assert:
			auto message = CreateMessage(mode, observerHeight);
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
		auto pObserver = CreateCacheBlockTouchObserver<TouchableCache>("Foo", Receipt_Type_Marker);

		// Assert:
		EXPECT_EQ("FooTouchObserver", pObserver->name());
	}

	TEST(TEST_CLASS, CacheBlockTouchObserverTouches_Commit) {
		// Arrange:
		auto pObserver = CreateCacheBlockTouchObserver<TouchableCache>("Foo", Receipt_Type_Marker);

		// Act + Assert:
		auto mode = NotifyMode::Commit;
		AssertTouching(*pObserver, mode, Height(1), Height(1), {});
		AssertTouching(*pObserver, mode, Height(11), Height(11), { 10 });
		AssertTouching(*pObserver, mode, Height(50), Height(50), { 10, 20, 30, 40 });
	}

	TEST(TEST_CLASS, CacheBlockTouchObserverTouches_Rollback) {
		// Arrange:
		auto pObserver = CreateCacheBlockTouchObserver<TouchableCache>("Foo", Receipt_Type_Marker);

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
		auto pObserver = CreateCacheBlockTouchObserver<TouchableCache>("Foo", Receipt_Type_Marker, BlockDuration(100));

		// Assert:
		EXPECT_EQ("FooTouchObserver", pObserver->name());
	}

	TEST(TEST_CLASS, NonzeroGracePeriod_CacheBlockTouchObserverTouches_Commit) {
		// Arrange:
		auto pObserver = CreateCacheBlockTouchObserver<TouchableCache>("Foo", Receipt_Type_Marker, BlockDuration(10));

		// Act + Assert:
		auto mode = NotifyMode::Commit;
		AssertTouching(*pObserver, mode, Height(1), Height(11), { 10 });
		AssertTouching(*pObserver, mode, Height(11), Height(21), { 10, 20 });
		AssertTouching(*pObserver, mode, Height(43), Height(53), { 10, 20, 30, 40, 50 });
		AssertTouching(*pObserver, mode, Height(90), Height(100), { 10, 20, 30, 40, 50, 60, 70, 80, 90 });
	}

	TEST(TEST_CLASS, NonzeroGracePeriod_CacheBlockTouchObserverTouches_Rollback) {
		// Arrange:
		auto pObserver = CreateCacheBlockTouchObserver<TouchableCache>("Foo", Receipt_Type_Marker, BlockDuration(10));

		// Act + Assert:
		auto mode = NotifyMode::Rollback;
		AssertTouching(*pObserver, mode, Height(1), Height(11));
		AssertTouching(*pObserver, mode, Height(11), Height(21));
		AssertTouching(*pObserver, mode, Height(43), Height(53));
		AssertTouching(*pObserver, mode, Height(90), Height(100));
	}

	// endregion
}}
