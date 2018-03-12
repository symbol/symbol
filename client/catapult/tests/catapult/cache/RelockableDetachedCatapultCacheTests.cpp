#include "catapult/cache/RelockableDetachedCatapultCache.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS RelockableDetachedCatapultCacheTests

	namespace {
		void SetHeight(CatapultCache& cache, Height height) {
			auto delta = cache.createDelta();
			cache.commit(height);
		}
	}

	TEST(TEST_CLASS, CanCreateRelockableDetachedCatapultCache) {
		// Arrange:
		auto cache = test::CreateEmptyCatapultCache();
		SetHeight(cache, Height(7));

		// Act:
		RelockableDetachedCatapultCache detachedCatapultCache(cache);

		// Assert: the detached cache should have the correct height and be lockable
		EXPECT_EQ(Height(7), detachedCatapultCache.height());
		EXPECT_TRUE(!!detachedCatapultCache.getAndLock());
	}

	TEST(TEST_CLASS, RelockableDetachedCatapultCacheIsInvalidatedWhenUnderlyingCacheChanges) {
		// Arrange:
		auto cache = test::CreateEmptyCatapultCache();
		SetHeight(cache, Height(7));

		// - create the detached cache
		RelockableDetachedCatapultCache detachedCatapultCache(cache);

		// Act: invalidate it
		SetHeight(cache, Height(11));

		// Assert: the detached cache should have the original height and not be lockable
		EXPECT_EQ(Height(7), detachedCatapultCache.height());
		EXPECT_FALSE(!!detachedCatapultCache.getAndLock());
	}

	TEST(TEST_CLASS, RelockableDetachedCatapultCacheCanBeRebasedOnTopOfUnderlyingCache) {
		// Arrange:
		auto cache = test::CreateEmptyCatapultCache();
		SetHeight(cache, Height(7));

		// - create the detached cache
		RelockableDetachedCatapultCache detachedCatapultCache(cache);

		// - invalidate it
		SetHeight(cache, Height(11));

		// Act: rebase it
		auto pDelta = detachedCatapultCache.rebaseAndLock();

		// Assert: the detached cache should have the new height and be lockable
		EXPECT_EQ(Height(11), detachedCatapultCache.height());
		EXPECT_TRUE(!!detachedCatapultCache.getAndLock());

		// - the returned delta should always be a valid pointer
		EXPECT_TRUE(!!pDelta);
	}
}}
