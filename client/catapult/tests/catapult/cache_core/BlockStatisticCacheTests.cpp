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

#include "catapult/cache_core/BlockStatisticCache.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"
#include <vector>

namespace catapult { namespace cache {

#define TEST_CLASS BlockStatisticCacheTests

	namespace {
		state::BlockStatistic CreateStatistic(uint32_t seed) {
			return state::BlockStatistic(Height(seed), Timestamp(seed), Difficulty(seed), BlockFeeMultiplier(seed));
		}
	}

	// region mixin traits based tests

	namespace {
		struct BlockStatisticCacheMixinTraits {
			class CacheType : public BlockStatisticCache {
			public:
				CacheType() : BlockStatisticCache(300)
				{}
			};

			using IdType = state::BlockStatistic;
			using ValueType = state::BlockStatistic;

			static uint8_t GetRawId(const IdType& id) {
				return static_cast<uint8_t>(id.Height.unwrap());
			}

			static const IdType& GetId(const ValueType& value) {
				return value;
			}

			static IdType MakeId(uint8_t id) {
				return CreateStatistic(id);
			}

			static ValueType CreateWithId(uint8_t id) {
				return MakeId(id);
			}
		};

		// custom modification policy is needed because elements are ordered
		struct BlockStatisticCacheDeltaModificationPolicy : public test::DeltaRemoveInsertModificationPolicy {
			static constexpr auto Is_Mutable = true;
			static constexpr auto Is_Strictly_Ordered = true;
		};
	}

	DEFINE_CACHE_CONTAINS_TESTS_ORDERING(BlockStatisticCacheMixinTraits, ViewAccessor, Strictly_Ordered, _View)
	DEFINE_CACHE_CONTAINS_TESTS_ORDERING(BlockStatisticCacheMixinTraits, DeltaAccessor, Strictly_Ordered, _Delta)

	DEFINE_CACHE_ITERATION_TESTS_ORDERING(BlockStatisticCacheMixinTraits, ViewAccessor, Strictly_Ordered, _View)
	DEFINE_CACHE_ITERATION_TESTS_ORDERING(BlockStatisticCacheMixinTraits, DeltaAccessor, Strictly_Ordered, _Delta)

	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(BlockStatisticCacheMixinTraits, BlockStatisticCacheDeltaModificationPolicy, _Delta)

	DEFINE_CACHE_BASIC_TESTS(BlockStatisticCacheMixinTraits,)

	// endregion

	// *** custom tests ***

	namespace {
		void SeedCache(BlockStatisticCacheDelta& delta, size_t count, uint32_t startHeight) {
			for (auto i = 0u; i < count; ++i)
				delta.insert(CreateStatistic(startHeight + i));
		}

		void SeedCache(BlockStatisticCache& cache, size_t count, uint32_t startHeight = 1) {
			auto delta = cache.createDelta();
			SeedCache(*delta, count, startHeight);
			cache.commit();
		}
	}

	// region insert

	TEST(TEST_CLASS, InsertThrowsWhenElementHasUnexpectedHeight) {
		// Arrange:
		BlockStatisticCache cache(300);
		SeedCache(cache, 10);
		auto delta = cache.createDelta();

		// Act + Assert:
		EXPECT_THROW(delta->insert(CreateStatistic(1)), catapult_invalid_argument);
		EXPECT_THROW(delta->insert(CreateStatistic(17)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanInsertElementWhenOriginalSetIsEmpty) {
		// Arrange:
		BlockStatisticCache cache(300);
		auto delta = cache.createDelta();

		// Act:
		delta->insert(CreateStatistic(1));
		delta->insert(CreateStatistic(2));

		// Assert:
		EXPECT_EQ(2u, delta->size());
		EXPECT_TRUE(delta->contains(CreateStatistic(1)));
		EXPECT_TRUE(delta->contains(CreateStatistic(2)));
	}

	TEST(TEST_CLASS, CanInsertElementWhenOriginalSetIsNotEmpty) {
		// Arrange:
		BlockStatisticCache cache(300);
		SeedCache(cache, 3);
		auto delta = cache.createDelta();

		// Act:
		delta->insert(CreateStatistic(4));
		delta->insert(CreateStatistic(5));

		// Assert:
		EXPECT_EQ(5u, delta->size());
		EXPECT_TRUE(delta->contains(CreateStatistic(4)));
		EXPECT_TRUE(delta->contains(CreateStatistic(5)));
	}

	TEST(TEST_CLASS, CanInsertElementAtArbitraryHeightWhenOriginalSetIsEmpty) {
		// Arrange:
		BlockStatisticCache cache(300);
		auto delta = cache.createDelta();

		// Act: add elements with heights starting at 117
		delta->insert(CreateStatistic(117));
		delta->insert(CreateStatistic(118));

		// Assert:
		EXPECT_EQ(2u, delta->size());
		EXPECT_TRUE(delta->contains(CreateStatistic(117)));
		EXPECT_TRUE(delta->contains(CreateStatistic(118)));
	}

	TEST(TEST_CLASS, CanInsertElementAtArbitraryHeightWhenOriginalSetIsEmptied) {
		// Arrange: create a delta around a cache with 3 elements
		BlockStatisticCache cache(300);
		SeedCache(cache, 3);
		auto delta = cache.createDelta();

		// Act: empty the cache
		delta->remove(CreateStatistic(3));
		delta->remove(CreateStatistic(2));
		delta->remove(CreateStatistic(1));

		// - add elements with heights starting at 211
		delta->insert(CreateStatistic(211));
		delta->insert(CreateStatistic(212));

		// Assert:
		EXPECT_EQ(2u, delta->size());
		EXPECT_TRUE(delta->contains(CreateStatistic(211)));
		EXPECT_TRUE(delta->contains(CreateStatistic(212)));
	}

	// endregion

	// region remove

	TEST(TEST_CLASS, CanRemoveElementWhenOriginalSetIsEmpty) {
		// Arrange:
		BlockStatisticCache cache(300);
		auto delta = cache.createDelta();
		auto element1 = CreateStatistic(1);
		auto element2 = CreateStatistic(2);
		delta->insert(element1);
		delta->insert(element2);

		// Sanity check:
		EXPECT_EQ(2u, delta->size());
		EXPECT_TRUE(delta->contains(element1));
		EXPECT_TRUE(delta->contains(element2));

		// Act:
		delta->remove(element2);

		// Assert:
		EXPECT_EQ(1u, delta->size());
		EXPECT_TRUE(delta->contains(element1));
		EXPECT_FALSE(delta->contains(element2));
	}

	TEST(TEST_CLASS, CanRemoveElementWhenOriginalSetIsNotEmpty) {
		// Arrange:
		BlockStatisticCache cache(300);
		SeedCache(cache, 3);
		auto delta = cache.createDelta();
		auto element = CreateStatistic(3);

		// Act:
		delta->remove(element);

		// Assert:
		EXPECT_EQ(2u, delta->size());
		EXPECT_FALSE(delta->contains(element));
	}

	TEST(TEST_CLASS, RemovingElementAndInsertingNewElementWithSameHeightUpdatesCacheWithNewElement) {
		// Arrange:
		BlockStatisticCache cache(300);
		SeedCache(cache, 3);
		auto element = CreateStatistic(3);
		auto newElement = CreateStatistic(3);
		newElement.Timestamp = Timestamp(123);

		// Sanity: initial timestamp is 3
		{
			auto view = cache.createView();
			auto statisticRange = view->statistics(Height(3), 1);
			auto foundElement = *statisticRange.begin();
			EXPECT_EQ(Height(3), foundElement.Height);
			EXPECT_EQ(Timestamp(3), foundElement.Timestamp);
		}

		// Act:
		{
			auto delta = cache.createDelta();
			delta->remove(element);
			delta->insert(newElement);
			cache.commit();
		}

		auto view = cache.createView();
		auto statisticRange = view->statistics(Height(3), 1);
		auto foundElement = *statisticRange.begin();

		// Assert: new timestamp is 123
		EXPECT_EQ(3u, view->size());
		EXPECT_EQ(Height(3), foundElement.Height);
		EXPECT_EQ(Timestamp(123), foundElement.Timestamp);
	}

	TEST(TEST_CLASS, RemoveThrowsWhenElementHasUnexpectedHeight) {
		// Arrange:
		BlockStatisticCache cache(300);
		SeedCache(cache, 10);
		auto delta = cache.createDelta();

		// Act + Assert:
		EXPECT_THROW(delta->remove(CreateStatistic(5)), catapult_invalid_argument);
		EXPECT_THROW(delta->remove(CreateStatistic(17)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, RemoveThrowsWhenCacheIsEmpty) {
		// Arrange:
		BlockStatisticCache cache(300);
		auto delta = cache.createDelta();

		// Act + Assert:
		EXPECT_THROW(delta->remove(CreateStatistic(5)), catapult_runtime_error);
		EXPECT_THROW(delta->remove(CreateStatistic(17)), catapult_runtime_error);
	}

	// endregion

	// region prune

	TEST(TEST_CLASS, PruningBoundaryIsInitiallyUnset) {
		// Arrange:
		BlockStatisticCache cache(300);
		auto delta = cache.createDelta();

		// Assert:
		EXPECT_FALSE(delta->pruningBoundary().isSet());
	}

	TEST(TEST_CLASS, PruneUpdatesPruningBoundary) {
		// Arrange:
		BlockStatisticCache cache(420);
		auto delta = cache.createDelta();

		// Act:
		delta->prune(Height(123 + 420));
		auto pruningBoundary = delta->pruningBoundary();

		// Assert:
		EXPECT_TRUE(delta->pruningBoundary().isSet());
		auto statistic = pruningBoundary.value();
		EXPECT_EQ(Height(123), statistic.Height);
		EXPECT_EQ(Timestamp(0), statistic.Timestamp);
		EXPECT_EQ(Difficulty(0), statistic.Difficulty);
		EXPECT_EQ(BlockFeeMultiplier(0), statistic.FeeMultiplier);
	}

	// endregion

	// region statistics

	namespace {
		struct ViewTraits {
			static auto SeedAndCreateView(BlockStatisticCache& cache, uint32_t count) {
				SeedCache(cache, count);
				return cache.createView();
			}
		};

		struct DeltaTraits {
			static auto SeedAndCreateView(BlockStatisticCache& cache, uint32_t count) {
				// seed a mix of committed and added statistics (this indirectly tests that iterator returns data in correct order)
				auto numAddedStatistics = std::min<uint32_t>(25, count);
				auto numCommittedStatistics = count - numAddedStatistics;

				SeedCache(cache, numCommittedStatistics);

				auto delta = cache.createDelta();
				SeedCache(*delta, numAddedStatistics, numCommittedStatistics + 1);
				return delta;
			}
		};
	}

#define VIEW_OR_DELTA_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_View) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ViewTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Delta) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DeltaTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	VIEW_OR_DELTA_TEST(StatisticsReturnsExpectedRange_NotIncludingLargestHeight) {
		// Arrange:
		BlockStatisticCache cache(300);
		auto view = TTraits::SeedAndCreateView(cache, 100);

		auto beginStatistic = state::BlockStatistic(Height(78 - 35 + 1));
		auto endStatistic = state::BlockStatistic(Height(79));

		// Act:
		auto statisticRange = view->statistics(Height(78), 35);

		// Assert:
		EXPECT_EQ(beginStatistic, *statisticRange.begin());
		EXPECT_EQ(endStatistic, *statisticRange.end());
	}

	VIEW_OR_DELTA_TEST(StatisticsReturnsExpectedRange_IncludingLargestHeight) {
		// Arrange:
		BlockStatisticCache cache(300);
		auto view = TTraits::SeedAndCreateView(cache, 100);

		auto beginStatistic = state::BlockStatistic(Height(100 - 35 + 1));

		// Act:
		auto statisticRange = view->statistics(Height(100), 35);

		// Assert:
		EXPECT_EQ(beginStatistic, *statisticRange.begin());
		EXPECT_EQ(view->tryMakeIterableView()->end(), statisticRange.end());
	}

	VIEW_OR_DELTA_TEST(StatisticsFirstIteratorPointsToSmallestStatisticWhenNotEnoughStatisticsAreAvailable) {
		// Arrange:
		BlockStatisticCache cache(300);
		auto view = TTraits::SeedAndCreateView(cache, 100);

		auto beginStatistic = state::BlockStatistic(Height(1));
		auto endStatistic = state::BlockStatistic(Height(31));

		// Act:
		auto statisticRange = view->statistics(Height(30), 100);

		// Assert:
		EXPECT_EQ(beginStatistic, *statisticRange.begin());
		EXPECT_EQ(endStatistic, *statisticRange.end());
	}

	VIEW_OR_DELTA_TEST(StatisticsThrowsWhenCacheIsEmpty) {
		// Arrange:
		BlockStatisticCache cache(300);
		auto view = TTraits::SeedAndCreateView(cache, 0);

		// Act + Assert:
		EXPECT_THROW(view->statistics(Height(78), 35), catapult_runtime_error);
	}

	VIEW_OR_DELTA_TEST(StatisticsThrowsWhenHeightOrCountIsZero) {
		// Arrange:
		BlockStatisticCache cache(300);
		auto view = TTraits::SeedAndCreateView(cache, 100);

		// Act + Assert:
		EXPECT_THROW(view->statistics(Height(0), 1), catapult_invalid_argument);
		EXPECT_THROW(view->statistics(Height(50), 0), catapult_invalid_argument);
	}

	VIEW_OR_DELTA_TEST(StatisticsThrowsWhenStatisticWithSpecifiedHeightIsNotFound) {
		// Arrange:
		BlockStatisticCache cache(300);
		auto view = TTraits::SeedAndCreateView(cache, 100);

		// Act + Assert:
		EXPECT_THROW(view->statistics(Height(101), 1), catapult_invalid_argument);
		EXPECT_THROW(view->statistics(Height(102), 1), catapult_invalid_argument);
		EXPECT_THROW(view->statistics(Height(1000), 1), catapult_invalid_argument);
	}

	// endregion
}}
