#include "catapult/validators/ValidatorContext.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS ValidatorContextTests

	TEST(TEST_CLASS, CanCreateValidatorContextAroundHeightAndNetworkAndCache) {
		// Act:
		auto networkInfo = model::NetworkInfo(static_cast<model::NetworkIdentifier>(0xAD), {}, {});
		auto cache = test::CreateEmptyCatapultCache();
		auto cacheView = cache.createView();
		auto readOnlyCache = cacheView.toReadOnly();
		auto context = ValidatorContext(Height(1234), Timestamp(987), networkInfo, readOnlyCache);

		// Assert:
		EXPECT_EQ(Height(1234), context.Height);
		EXPECT_EQ(Timestamp(987), context.BlockTime);
		EXPECT_EQ(static_cast<model::NetworkIdentifier>(0xAD), context.Network.Identifier);
		EXPECT_EQ(&readOnlyCache, &context.Cache);
	}
}}
