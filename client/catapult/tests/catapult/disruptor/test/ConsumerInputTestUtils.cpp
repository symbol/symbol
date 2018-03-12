#include "ConsumerInputTestUtils.h"

namespace catapult { namespace test {

	void AssertEmptyInput(disruptor::ConsumerInput& input) {
		EXPECT_TRUE(input.empty());
		EXPECT_FALSE(input.hasBlocks());
		EXPECT_FALSE(input.hasTransactions());

		EXPECT_EQ(disruptor::InputSource::Unknown, input.source());
		EXPECT_THROW(input.blocks(), catapult_runtime_error);
		EXPECT_THROW(input.transactions(), catapult_runtime_error);
	}
}}
