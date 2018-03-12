#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS ReadOnlyArtifactCacheTests

	using ReadOnlyArtifactCacheType = ReadOnlyArtifactCache<test::BasicSimpleCacheView, test::BasicSimpleCacheDelta, size_t, size_t>;

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
		// Assert:
		RunReadOnlyViewTest([](const auto& readOnlyCache) {
			EXPECT_TRUE(readOnlyCache.contains(1));
			EXPECT_TRUE(readOnlyCache.contains(2));
			EXPECT_FALSE(readOnlyCache.contains(3));
			EXPECT_FALSE(readOnlyCache.contains(4));
			EXPECT_FALSE(readOnlyCache.contains(5));
		});
	}

	TEST(TEST_CLASS, ReadOnlyDeltaContainsBothCommittedAndUncommittedElements) {
		// Assert:
		RunReadOnlyDeltaTest([](const auto& readOnlyCache) {
			EXPECT_TRUE(readOnlyCache.contains(1));
			EXPECT_TRUE(readOnlyCache.contains(2));
			EXPECT_TRUE(readOnlyCache.contains(3));
			EXPECT_TRUE(readOnlyCache.contains(4));
			EXPECT_FALSE(readOnlyCache.contains(5));
		});
	}

	// endregion

	// region get

	TEST(TEST_CLASS, ReadOnlyViewCanAccessCommittedElementsViaGet) {
		// Assert:
		RunReadOnlyViewTest([](const auto& readOnlyCache) {
			EXPECT_EQ(1u, readOnlyCache.get(1));
			EXPECT_EQ(4u, readOnlyCache.get(2));
		});
	}

	TEST(TEST_CLASS, ReadOnlyViewCannotAccessUncommittedElementsViaGet) {
		// Act + Assert:
		RunReadOnlyViewTest([](const auto& readOnlyCache) {
			EXPECT_THROW(readOnlyCache.get(3), catapult_out_of_range);
			EXPECT_THROW(readOnlyCache.get(4), catapult_out_of_range);
		});
	}

	TEST(TEST_CLASS, ReadOnlyDeltaCanAccessBothCommittedAndUncommittedElementsViaGet) {
		// Assert:
		RunReadOnlyDeltaTest([](const auto& readOnlyCache) {
			EXPECT_EQ(1u, readOnlyCache.get(1));
			EXPECT_EQ(4u, readOnlyCache.get(2));
			EXPECT_EQ(9u, readOnlyCache.get(3));
			EXPECT_EQ(16u, readOnlyCache.get(4));
		});
	}

	// endregion

	// region isActive

	TEST(TEST_CLASS, ReadOnlyViewCanDetermineIfArtifactIsActive) {
		// Assert:
		RunReadOnlyViewTest([](const auto& readOnlyCache) {
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

	TEST(TEST_CLASS, ReadOnlyDeltaCanDetermineIfArtifactIsActive) {
		// Assert:
		RunReadOnlyDeltaTest([](const auto& readOnlyCache) {
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
