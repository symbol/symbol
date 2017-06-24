#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	TEST(ReadOnlyCatapultCacheTests, CanCreateAroundArbitraryCaches) {
		// Arrange:
		test::SimpleCacheT<0> cache0;
		test::SimpleCacheT<2> cache2;
		std::vector<const void*> subViews = {
			&cache0.createView()->asReadOnly(),
			nullptr,
			&cache2.createView()->asReadOnly()
		};

		// Act:
		ReadOnlyCatapultCache readOnlyCache(subViews);

		// Assert:
		// - subcaches match input
		EXPECT_EQ(subViews[0], &readOnlyCache.sub<test::SimpleCacheT<0>>());
		EXPECT_EQ(subViews[2], &readOnlyCache.sub<test::SimpleCacheT<2>>());
	}
}}
