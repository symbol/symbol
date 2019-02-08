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

#pragma once
#include "CacheMixinsTests.h"

namespace catapult { namespace test {

	/// Test suite for cache prune functionality.
	template<typename TTraits>
	class CachePruneTests {
	private:
		using CacheType = typename TTraits::CacheType;

	private:
		static constexpr auto NumSeedElements() {
			return 10u;
		}

	public:
		static void AssertPruneIsNoOpWhenCacheIsEmpty() {
			// Arrange:
			CacheType cache;
			auto delta = cache.createDelta();

			// Sanity:
			EXPECT_EQ(0u, delta->size());

			// Act: prune on empty cache does not throw
			delta->prune(Height(123));

			// Assert:
			EXPECT_EQ(0u, delta->size());
		}

		static void AssertPruneIsNoOpWhenAllExpirationsAreAfterPruneHeight() {
			// Arrange:
			CacheType cache;
			SeedCache(cache);
			auto delta = cache.createDelta();

			// Act: first element expires at height 10
			for (auto i = 0u; 10u > i; ++i)
				delta->prune(Height(i));

			// Assert: size has not changed
			EXPECT_EQ(NumSeedElements(), delta->size());
		}

		static void AssertPruneIsNoOpWhenNoElementsExpireAtExactPruneHeight() {
			// Arrange:
			CacheType cache;
			SeedCache(cache);
			auto delta = cache.createDelta();

			// Act: for heights 11 to 19 no elements expired
			for (auto i = 11u; 20u > i; ++i)
				delta->prune(Height(i));

			// Assert: size has not changed
			EXPECT_EQ(NumSeedElements(), delta->size());
		}

		static void AssertCanPruneElementsAtSingleHeight() {
			// Arrange:
			CacheType cache;
			SeedCache(cache);
			auto delta = cache.createDelta();

			// Act: prune element with id 3 that expires at height 30
			delta->prune(Height(30));
			cache.commit();

			// Assert:
			auto view = cache.createView();
			EXPECT_EQ(9u, view->size());
			for (auto id : std::initializer_list<uint8_t>{ 1, 2, 4, 5, 6, 7, 8, 9, 10 })
				EXPECT_TRUE(view->contains(TTraits::MakeId(id))) << "id " << static_cast<uint16_t>(id);
		}

		static void AssertCanPruneElementsAtMultipleHeights() {
			// Arrange:
			CacheType cache;
			SeedCache(cache);
			auto delta = cache.createDelta();

			// Act: prune elements with id 1 - 7 that expire at heights 10, 20, ..., 70
			PruneAll(*delta, Height(70));
			cache.commit();

			// Assert:
			auto view = cache.createView();
			EXPECT_EQ(3u, view->size());
			for (auto id : std::initializer_list<uint8_t>{ 8, 9, 10 })
				EXPECT_TRUE(view->contains(TTraits::MakeId(id))) << "id " << static_cast<uint16_t>(id);
		}

		static void AssertCanPruneMultipleElementsAtMultipleHeights() {
			// Arrange:
			CacheType cache;
			SeedCacheWithMultipleIdsAtSameHeight(cache);
			auto delta = cache.createDelta();

			// Act: prune elements expiring at height 10, 20 ids: 1, 2, 5, 6, 9, 10
			PruneAll(*delta, Height(20));
			cache.commit();

			// Assert:
			auto view = cache.createView();
			EXPECT_EQ(4u, view->size());
			for (auto id : std::initializer_list<uint8_t>{ 3, 4, 7, 8 })
				EXPECT_TRUE(view->contains(TTraits::MakeId(id))) << "id " << static_cast<uint16_t>(id);
		}

		static void AssertCanPruneAllElements() {
			// Arrange:
			CacheType cache;
			SeedCache(cache);
			auto delta = cache.createDelta();

			// Act: prune all elements
			PruneAll(*delta, Height(100));

			// Assert:
			EXPECT_EQ(0u, delta->size());
		}

		static void AssertPruneIsIdempotent() {
			// Arrange:
			CacheType cache;
			SeedCache(cache);
			auto delta = cache.createDelta();

			// Act: prune elements with id 1 - 5 (multiple times)
			for (auto i = 0u; 10 > i; ++i)
				PruneAll(*delta, Height(50));

			cache.commit();

			// Assert:
			auto view = cache.createView();
			EXPECT_EQ(5u, view->size());
			for (auto id : std::initializer_list<uint8_t>{ 6, 7, 8, 9, 10 })
				EXPECT_TRUE(view->contains(TTraits::MakeId(id))) << "id " << static_cast<uint16_t>(id);
		}

	public:
		template<typename TAction>
		static void RunCustomPruneTest(TAction action) {
			// Arrange:
			CacheType cache;
			SeedCache(cache);
			auto delta = cache.createDelta();

			// Act + Assert:
			action(*delta, [&cache, &delta](auto height) {
				PruneAll(*delta, height);
				cache.commit();
			});
		}

	private:
		static void PruneAll(typename CacheType::CacheDeltaType& delta, Height maxHeight) {
			// Act: increment by 10 (this is required for caches that prune at exact heights)
			for (auto height = Height(10); height <= maxHeight; height = height + Height(10))
				delta.prune(height);
		}

		static void SeedCache(CacheType& cache) {
			auto delta = cache.createDelta();
			for (uint8_t i = 1; i <= NumSeedElements(); ++i)
				delta->insert(TTraits::CreateWithIdAndExpiration(i, Height(10 * i)));

			// Sanity:
			EXPECT_EQ(NumSeedElements(), delta->size());

			cache.commit();
		}

		static void SeedCacheWithMultipleIdsAtSameHeight(CacheType& cache) {
			auto delta = cache.createDelta();
			// 10, 20, 30, 40, 10, 20, 30, 40, 10, 20
			for (uint8_t i = 1; i <= NumSeedElements(); ++i)
				delta->insert(TTraits::CreateWithIdAndExpiration(i, Height(10 * i % 40)));

			// Sanity:
			EXPECT_EQ(NumSeedElements(), delta->size());

			cache.commit();
		}
	};
}}

#define MAKE_CACHE_PRUNE_TEST(CACHE_TRAITS, SUFFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##SUFFIX) { test::CachePruneTests<CACHE_TRAITS>::Assert##TEST_NAME(); }

#define DEFINE_CACHE_PRUNE_TESTS(CACHE_TRAITS, SUFFIX) \
	MAKE_CACHE_PRUNE_TEST(CACHE_TRAITS, SUFFIX, PruneIsNoOpWhenCacheIsEmpty) \
	MAKE_CACHE_PRUNE_TEST(CACHE_TRAITS, SUFFIX, PruneIsNoOpWhenAllExpirationsAreAfterPruneHeight) \
	MAKE_CACHE_PRUNE_TEST(CACHE_TRAITS, SUFFIX, PruneIsNoOpWhenNoElementsExpireAtExactPruneHeight) \
	MAKE_CACHE_PRUNE_TEST(CACHE_TRAITS, SUFFIX, CanPruneElementsAtSingleHeight) \
	MAKE_CACHE_PRUNE_TEST(CACHE_TRAITS, SUFFIX, CanPruneElementsAtMultipleHeights) \
	MAKE_CACHE_PRUNE_TEST(CACHE_TRAITS, SUFFIX, CanPruneMultipleElementsAtMultipleHeights) \
	MAKE_CACHE_PRUNE_TEST(CACHE_TRAITS, SUFFIX, CanPruneAllElements) \
	MAKE_CACHE_PRUNE_TEST(CACHE_TRAITS, SUFFIX, PruneIsIdempotent)
