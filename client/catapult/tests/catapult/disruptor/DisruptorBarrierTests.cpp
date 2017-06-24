#include "catapult/disruptor/DisruptorBarrier.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

	TEST(DisruptorBarrierTests, CanCreateBarrier) {
		// Arrange:
		DisruptorBarrier barrier(100, 1);

		// Assert:
		EXPECT_EQ(100u, barrier.level());
		EXPECT_EQ(1u, barrier.position());
	}

	TEST(DisruptorBarrierTests, CanAdvanceBarrier) {
		// Arrange:
		DisruptorBarrier barrier(100, 1);

		// Act:
		barrier.advance();

		// Assert:
		EXPECT_EQ(100u, barrier.level());
		EXPECT_EQ(2u, barrier.position());
	}
}}
