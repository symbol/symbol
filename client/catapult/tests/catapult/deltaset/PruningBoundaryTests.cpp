#include "catapult/deltaset/PruningBoundary.h"
#include "tests/TestHarness.h"

namespace catapult { namespace deltaset {

	TEST(PruningBoundaryTests, CanCreateUnsetPruningBoundary) {
		// Act:
		PruningBoundary<int> boundary;

		// Assert:
		EXPECT_FALSE(boundary.isSet());
	}

	TEST(PruningBoundaryTests, CanCreateUnsetPruningBoundary_SharedPtr) {
		// Act:
		PruningBoundary<std::shared_ptr<int>> boundary;

		// Assert:
		EXPECT_FALSE(boundary.isSet());
	}

	TEST(PruningBoundaryTests, CanCreatePruningBoundaryWithValue) {
		// Act:
		PruningBoundary<int> boundary(17);

		// Assert:
		EXPECT_TRUE(boundary.isSet());
		EXPECT_EQ(17, boundary.value());
	}

	TEST(PruningBoundaryTests, CanCreatePruningBoundaryWithValue_SharedPtr) {
		// Act:
		PruningBoundary<std::shared_ptr<int>> boundary(std::make_shared<int>(17));

		// Assert:
		EXPECT_TRUE(boundary.isSet());
		EXPECT_EQ(17, *boundary.value());
	}
}}
