#include "catapult/thread/Task.h"
#include "tests/TestHarness.h"

namespace catapult { namespace thread {

#define TEST_CLASS TaskTests

	TEST(TEST_CLASS, CanCreateDefaultTask) {
		// Act:
		Task task;

		// Assert:
		EXPECT_TRUE(task.Name.empty());
		EXPECT_EQ(utils::TimeSpan(), task.StartDelay);
		EXPECT_EQ(utils::TimeSpan(), task.RepeatDelay);

		EXPECT_FALSE(!!task.Callback);
	}

	TEST(TEST_CLASS, CanCreateNamedTask) {
		// Act:
		auto counter = 0u;
		auto task = CreateNamedTask("foo task", [&counter]() {
			++counter;
			return thread::make_ready_future(thread::TaskResult::Continue);
		});

		// Assert:
		EXPECT_EQ("foo task", task.Name);
		EXPECT_EQ(utils::TimeSpan(), task.StartDelay);
		EXPECT_EQ(utils::TimeSpan(), task.RepeatDelay);

		// - execute the callback (use counter as a proxy that the callback is set correctly)
		ASSERT_TRUE(!!task.Callback);
		task.Callback();
		EXPECT_EQ(1u, counter);
	}
}}
