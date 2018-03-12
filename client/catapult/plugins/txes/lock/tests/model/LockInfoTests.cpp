#include "src/model/LockInfo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS LockInfoTests

	TEST(TEST_CLASS, IsActiveReturnsTrueIfHeightIsLessThanLockInfoHeight) {
		// Arrange:
		HashLockInfo lockInfo;
		lockInfo.Height = Height(123);

		// Act + Assert:
		EXPECT_TRUE(lockInfo.isActive(Height(122)));
		EXPECT_TRUE(lockInfo.isActive(Height(1)));
	}

	TEST(TEST_CLASS, IsActiveReturnsFalseIfHeightIsGreaterThanOrEqualToLockInfoHeight) {
		// Arrange:
		HashLockInfo lockInfo;
		lockInfo.Height = Height(123);

		// Act + Assert:
		EXPECT_FALSE(lockInfo.isActive(Height(123)));
		EXPECT_FALSE(lockInfo.isActive(Height(124)));
		EXPECT_FALSE(lockInfo.isActive(Height(1111)));
	}
}}
