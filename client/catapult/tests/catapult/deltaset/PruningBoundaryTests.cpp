#include "catapult/deltaset/PruningBoundary.h"
#include "tests/TestHarness.h"

namespace catapult { namespace deltaset {

#define TEST_CLASS PruningBoundaryTests

	TEST(TEST_CLASS, CanCreateUnsetPruningBoundary) {
		// Act:
		PruningBoundary<int> boundary;

		// Assert:
		EXPECT_FALSE(boundary.isSet());
	}

	TEST(TEST_CLASS, CanCreateUnsetPruningBoundary_SharedPtr) {
		// Act:
		PruningBoundary<std::shared_ptr<int>> boundary;

		// Assert:
		EXPECT_FALSE(boundary.isSet());
	}

	TEST(TEST_CLASS, CanCreatePruningBoundaryWithValue) {
		// Act:
		PruningBoundary<int> boundary(17);

		// Assert:
		EXPECT_TRUE(boundary.isSet());
		EXPECT_EQ(17, boundary.value());
	}

	TEST(TEST_CLASS, CanCreatePruningBoundaryWithValue_SharedPtr) {
		// Act:
		PruningBoundary<std::shared_ptr<int>> boundary(std::make_shared<int>(17));

		// Assert:
		EXPECT_TRUE(boundary.isSet());
		EXPECT_EQ(17, *boundary.value());
	}
}}
