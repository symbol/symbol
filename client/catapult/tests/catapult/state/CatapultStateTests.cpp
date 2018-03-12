#include "catapult/state/CatapultState.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS CatapultStateTests

	TEST(TEST_CLASS, CanCreateDefaultState) {
		// Act:
		CatapultState state;

		// Assert:
		EXPECT_EQ(model::ImportanceHeight(0), state.LastRecalculationHeight);
	}
}}
