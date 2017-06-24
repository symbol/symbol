#include "catapult/state/CatapultState.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

	TEST(CatapultStateTests, CanCreateDefaultState) {
		// Act:
		CatapultState state;

		// Assert:
		EXPECT_EQ(model::ImportanceHeight(0), state.LastRecalculationHeight);
	}
}}
