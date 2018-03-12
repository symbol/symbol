#include "catapult/state/AccountState.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountStateTests

	TEST(TEST_CLASS, AccountStateCtorInitializesProperFields) {
		// Arrange:
		auto address = test::GenerateRandomAddress();
		auto height = Height(1234);

		// Act:
		AccountState state(address, height);

		// Assert:
		EXPECT_EQ(address, state.Address);
		EXPECT_EQ(height, state.AddressHeight);
		EXPECT_EQ(Height(0), state.PublicKeyHeight);
		EXPECT_EQ(0u, state.Balances.size());

		for (const auto& pair : state.ImportanceInfo) {
			EXPECT_EQ(Importance(0), pair.Importance);
			EXPECT_EQ(model::ImportanceHeight(0), pair.Height);
		}
	}
}}
