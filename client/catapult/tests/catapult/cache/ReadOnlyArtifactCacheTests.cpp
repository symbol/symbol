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

#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS ReadOnlyArtifactCacheTests

	using ReadOnlyArtifactCacheType = ReadOnlyArtifactCache<test::BasicSimpleCacheView, test::BasicSimpleCacheDelta, uint64_t, uint64_t>;

	namespace {
		template<typename TAssertFunc>
		void RunReadOnlyViewTest(TAssertFunc assertFunc) {
			// Arrange:
			test::SimpleCache cache;
			{
				auto cacheDelta = cache.createDelta();
				cacheDelta->increment(); // committed
				cacheDelta->increment();
				cache.commit();
				cacheDelta->increment(); // uncommitted
				cacheDelta->increment();
			}

			// Act:
			auto cacheView = cache.createView();
			ReadOnlyArtifactCacheType readOnlyCache(*cacheView);

			// Assert:
			EXPECT_EQ(2u, readOnlyCache.size());
			assertFunc(readOnlyCache);
		}

		template<typename TAssertFunc>
		void RunReadOnlyDeltaTest(TAssertFunc assertFunc) {
			// Arrange:
			test::SimpleCache cache;
			auto cacheDelta = cache.createDelta();
			cacheDelta->increment(); // committed
			cacheDelta->increment();
			cache.commit();
			cacheDelta->increment(); // uncommitted
			cacheDelta->increment();

			// Act:
			ReadOnlyArtifactCacheType readOnlyCache(*cacheDelta);

			// Assert:
			EXPECT_EQ(4u, readOnlyCache.size());
			assertFunc(readOnlyCache);
		}
	}

	// region contains

	TEST(TEST_CLASS, ReadOnlyViewOnlyContainsCommittedElements) {
		RunReadOnlyViewTest([](const auto& readOnlyCache) {
			EXPECT_TRUE(readOnlyCache.contains(1));
			EXPECT_TRUE(readOnlyCache.contains(2));
			EXPECT_FALSE(readOnlyCache.contains(3));
			EXPECT_FALSE(readOnlyCache.contains(4));
			EXPECT_FALSE(readOnlyCache.contains(5));
		});
	}

	TEST(TEST_CLASS, ReadOnlyDeltaContainsBothCommittedAndUncommittedElements) {
		RunReadOnlyDeltaTest([](const auto& readOnlyCache) {
			EXPECT_TRUE(readOnlyCache.contains(1));
			EXPECT_TRUE(readOnlyCache.contains(2));
			EXPECT_TRUE(readOnlyCache.contains(3));
			EXPECT_TRUE(readOnlyCache.contains(4));
			EXPECT_FALSE(readOnlyCache.contains(5));
		});
	}

	// endregion

	// region find

	TEST(TEST_CLASS, ReadOnlyViewCanAccessCommittedElementsViaGet) {
		RunReadOnlyViewTest([](const auto& readOnlyCache) {
			EXPECT_EQ(1u, readOnlyCache.find(1).get());
			EXPECT_EQ(4u, readOnlyCache.find(2).get());
		});
	}

	TEST(TEST_CLASS, ReadOnlyViewCannotAccessUncommittedElementsViaGet) {
		RunReadOnlyViewTest([](const auto& readOnlyCache) {
			EXPECT_THROW(readOnlyCache.find(3).get(), catapult_out_of_range);
			EXPECT_THROW(readOnlyCache.find(4).get(), catapult_out_of_range);
		});
	}

	TEST(TEST_CLASS, ReadOnlyDeltaCanAccessBothCommittedAndUncommittedElementsViaGet) {
		RunReadOnlyDeltaTest([](const auto& readOnlyCache) {
			EXPECT_EQ(1u, readOnlyCache.find(1).get());
			EXPECT_EQ(4u, readOnlyCache.find(2).get());
			EXPECT_EQ(9u, readOnlyCache.find(3).get());
			EXPECT_EQ(16u, readOnlyCache.find(4).get());
		});
	}

	// endregion

	// region isActive

	TEST(TEST_CLASS, ReadOnlyViewCanDetermineWhetherOrNotArtifactIsActive) {
		RunReadOnlyViewTest([](const auto& readOnlyCache) {
			// Assert: { 1, 2 } committed
			// - height 2 (ids % 2 are active)
			EXPECT_FALSE(readOnlyCache.isActive(1, Height(2)));
			EXPECT_TRUE(readOnlyCache.isActive(2, Height(2)));
			EXPECT_FALSE(readOnlyCache.isActive(3, Height(2)));
			EXPECT_FALSE(readOnlyCache.isActive(4, Height(2)));
			EXPECT_FALSE(readOnlyCache.isActive(5, Height(2)));

			// - height 1 (ids % 1 are active)
			EXPECT_TRUE(readOnlyCache.isActive(1, Height(1)));
			EXPECT_TRUE(readOnlyCache.isActive(2, Height(1)));
			EXPECT_FALSE(readOnlyCache.isActive(3, Height(1)));
			EXPECT_FALSE(readOnlyCache.isActive(4, Height(1)));
			EXPECT_FALSE(readOnlyCache.isActive(5, Height(1)));

			// - height 3 (ids % 3 are active)
			EXPECT_FALSE(readOnlyCache.isActive(2, Height(3)));
			EXPECT_FALSE(readOnlyCache.isActive(3, Height(3)));
			EXPECT_FALSE(readOnlyCache.isActive(4, Height(3)));
		});
	}

	TEST(TEST_CLASS, ReadOnlyDeltaCanDetermineWhetherOrNotArtifactIsActive) {
		RunReadOnlyDeltaTest([](const auto& readOnlyCache) {
			// Assert: { 1, 2 } committed, { 3, 4 } uncommitted
			// - height 2 (ids % 2 are active)
			EXPECT_FALSE(readOnlyCache.isActive(1, Height(2)));
			EXPECT_TRUE(readOnlyCache.isActive(2, Height(2)));
			EXPECT_FALSE(readOnlyCache.isActive(3, Height(2)));
			EXPECT_TRUE(readOnlyCache.isActive(4, Height(2)));
			EXPECT_FALSE(readOnlyCache.isActive(5, Height(2)));

			// - height 1 (ids % 1 are active)
			EXPECT_TRUE(readOnlyCache.isActive(1, Height(1)));
			EXPECT_TRUE(readOnlyCache.isActive(2, Height(1)));
			EXPECT_TRUE(readOnlyCache.isActive(3, Height(1)));
			EXPECT_TRUE(readOnlyCache.isActive(4, Height(1)));
			EXPECT_FALSE(readOnlyCache.isActive(5, Height(1)));

			// - height 3 (ids % 3 are active)
			EXPECT_FALSE(readOnlyCache.isActive(2, Height(3)));
			EXPECT_TRUE(readOnlyCache.isActive(3, Height(3)));
			EXPECT_FALSE(readOnlyCache.isActive(4, Height(3)));
		});
	}

	// endregion
}}
