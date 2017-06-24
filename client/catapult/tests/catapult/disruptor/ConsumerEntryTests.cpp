#include "catapult/disruptor/ConsumerEntry.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

	TEST(ConsumerEntryTests, CanCreateAnEntry) {
		// Arrange:
		auto level = 123u;
		ConsumerEntry consumer(level);

		// Assert:
		EXPECT_EQ(123u, consumer.level());
		EXPECT_EQ(0u, consumer.position());
	}

	TEST(ConsumerEntryTests, CanAdvanceConsumerPosition) {
		// Arrange:
		auto level = 123u;
		ConsumerEntry consumer(level);

		// Act:
		for (auto i = 0; i < 10; ++i)
			consumer.advance();

		// Assert:
		EXPECT_EQ(123u, consumer.level());
		EXPECT_EQ(10u, consumer.position());
	}
}}
