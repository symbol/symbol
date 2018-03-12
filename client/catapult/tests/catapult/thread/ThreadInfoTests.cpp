#include "catapult/thread/ThreadInfo.h"
#include "tests/TestHarness.h"
#include <thread>

namespace catapult { namespace thread {

#define TEST_CLASS ThreadInfoTests

	TEST(TEST_CLASS, CanSetThreadName) {
		// Arrange:
		std::string threadName;
		std::thread([&threadName] {
			// Act:
			SetThreadName("foo BAR 123");
			threadName = GetThreadName();
		}).join();

		// Assert:
		EXPECT_EQ("foo BAR 123", threadName);
	}

	TEST(TEST_CLASS, CanSetLongTruncatedThreadName) {
		// Arrange:
		std::string threadName;
		std::thread([&threadName] {
			// Act:
			SetThreadName(std::string(2 * GetMaxThreadNameLength(), 'a'));
			threadName = GetThreadName();
		}).join();

		// Assert: the long thread name is truncated
		EXPECT_EQ(std::string(GetMaxThreadNameLength(), 'a'), threadName);
	}
}}
