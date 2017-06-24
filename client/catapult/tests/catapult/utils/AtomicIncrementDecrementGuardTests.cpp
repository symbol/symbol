#include "catapult/utils/AtomicIncrementDecrementGuard.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

	TEST(AtomicIncrementDecrementGuardTests, ConstructorIncrementsAtomic) {
		// Arrange:
		std::atomic<int> value(7);

		// Act:
		AtomicIncrementDecrementGuard<int> guard(value);

		// Assert:
		EXPECT_EQ(8, value);
	}

	TEST(AtomicIncrementDecrementGuardTests, MakeFunctionIncrementsAtomic) {
		// Arrange:
		std::atomic<int> value(7);

		// Act:
		auto guard = MakeIncrementDecrementGuard(value);

		// Assert:
		EXPECT_EQ(8, value);
	}

	TEST(AtomicIncrementDecrementGuardTests, DestructorDecrementsAtomic) {
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
