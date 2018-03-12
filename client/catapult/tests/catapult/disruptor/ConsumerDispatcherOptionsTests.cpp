#include "catapult/disruptor/ConsumerDispatcherOptions.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

#define TEST_CLASS ConsumerDispatcherOptionsTests

	TEST(TEST_CLASS, CanCreateOptions) {
		// Act:
		auto options = ConsumerDispatcherOptions("foo dispatcher", 123);

		// Assert:
		EXPECT_EQ("foo dispatcher", options.DispatcherName);
		EXPECT_EQ(123u, options.DisruptorSize);
		EXPECT_EQ(1u, options.ElementTraceInterval);
		EXPECT_TRUE(options.ShouldThrowIfFull);
	}
}}
