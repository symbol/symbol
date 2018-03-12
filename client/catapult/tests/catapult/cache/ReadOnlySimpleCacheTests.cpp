#include "catapult/cache/ReadOnlySimpleCache.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS ReadOnlySimpleCacheTests

	TEST(TEST_CLASS, ReadOnlyViewOnlyContainsCommittedElements) {
		// Arrange:
		test::SimpleCache cache;
		{
			auto cacheDelta = cache.createDelta();
			cacheDelta->increment(); // committed
			cache.commit();
			cacheDelta->increment(); // uncommitted
		}

		// Act:
		auto cacheView = cache.createView();
		test::BasicSimpleCache::CacheReadOnlyType readOnlyCache(*cacheView);

		// Assert:
		EXPECT_EQ(1u, readOnlyCache.size());
		EXPECT_TRUE(readOnlyCache.contains(1));
		EXPECT_FALSE(readOnlyCache.contains(2));
		EXPECT_FALSE(readOnlyCache.contains(3));
	}

	TEST(TEST_CLASS, ReadOnlyDeltaContainsBothCommittedAndUncommittedElements) {
		// Arrange:
		test::SimpleCache cache;
		auto cacheDelta = cache.createDelta();
		cacheDelta->increment(); // committed
		cache.commit();
		cacheDelta->increment(); // uncommitted

		// Act:
		test::BasicSimpleCache::CacheReadOnlyType readOnlyCache(*cacheDelta);

		// Assert:
		EXPECT_EQ(2u, readOnlyCache.size());
		EXPECT_TRUE(readOnlyCache.contains(1));
		EXPECT_TRUE(readOnlyCache.contains(2));
		EXPECT_FALSE(readOnlyCache.contains(3));
	}
}}
