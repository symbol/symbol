#include "catapult/cache/CacheHeight.h"
#include "tests/test/nodeps/LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	TEST(CacheHeightTests, HeightIsInitiallyZero) {
		// Act:
		CacheHeight height;

		// Assert:
		EXPECT_EQ(Height(0), height.view().get());
	}

	TEST(CacheHeightTests, CanChangeHeight) {
		// Arrange:
		CacheHeight height;

		// Act:
		height.modifier().set(Height(343));

		// Assert:
		EXPECT_EQ(Height(343), height.view().get());
	}

	// region synchronization

	namespace {
		auto CreateLockProvider() {
			return std::make_unique<CacheHeight>();
		}
	}

	DEFINE_LOCK_PROVIDER_TESTS(CacheHeightTests)

	// endregion
}}
