#include "catapult/utils/AtomicIncrementDecrementGuard.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS AtomicIncrementDecrementGuardTests

	TEST(TEST_CLASS, ConstructorIncrementsAtomic) {
		// Arrange:
		std::atomic<int> value(7);

		// Act:
		AtomicIncrementDecrementGuard<int> guard(value);

		// Assert:
		EXPECT_EQ(8, value);
	}

	TEST(TEST_CLASS, MakeFunctionIncrementsAtomic) {
		// Arrange:
		std::atomic<int> value(7);

		// Act:
		auto guard = MakeIncrementDecrementGuard(value);

		// Assert:
		EXPECT_EQ(8, value);
	}

	TEST(TEST_CLASS, DestructorDecrementsAtomic) {
		// Arrange:
		std::atomic<int> value(7);

		// Act:
		{
			AtomicIncrementDecrementGuard<int> guard(value);
		}

		// Assert:
		EXPECT_EQ(7, value);
	}
}}
