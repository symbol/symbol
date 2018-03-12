#include "catapult/disruptor/DisruptorBarrier.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

#define TEST_CLASS DisruptorBarrierTests

	TEST(TEST_CLASS, CanCreateBarrier) {
		// Arrange:
		DisruptorBarrier barrier(100, 1);

		// Assert:
		EXPECT_EQ(100u, barrier.level());
		EXPECT_EQ(1u, barrier.position());
	}

	TEST(TEST_CLASS, CanAdvanceBarrier) {
		// Arrange:
		DisruptorBarrier barrier(100, 1);

		// Act:
		barrier.advance();

		// Assert:
		EXPECT_EQ(100u, barrier.level());
		EXPECT_EQ(2u, barrier.position());
	}
}}
