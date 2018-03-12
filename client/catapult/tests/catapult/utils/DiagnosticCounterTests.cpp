#include "catapult/utils/DiagnosticCounter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS DiagnosticCounterTests

	TEST(TEST_CLASS, CanCreateCounter) {
		// Act:
		DiagnosticCounter counter(DiagnosticCounterId("CAT"), []() { return 123; });

		// Assert:
		EXPECT_EQ("CAT", counter.id().name());
		EXPECT_EQ(123u, counter.value());
	}

	TEST(TEST_CLASS, CounterValueAccessesSupplierForLatestValue) {
		// Arrange:
		auto i = 0;
		DiagnosticCounter counter(DiagnosticCounterId(), [&i]() {
			++i;
			return i * i;
		});

		// Act:
		auto value1 = counter.value();
		auto value2 = counter.value();
		auto value3 = counter.value();

		// Assert:
		EXPECT_EQ(1u, value1);
		EXPECT_EQ(4u, value2);
		EXPECT_EQ(9u, value3);
	}
}}
