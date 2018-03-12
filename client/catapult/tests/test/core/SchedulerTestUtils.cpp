#include "SchedulerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	void AssertUnscheduledTask(const thread::Task& task, const std::string& expectedTaskName) {
		// Assert:
		EXPECT_EQ(expectedTaskName, task.Name) << expectedTaskName;
		EXPECT_EQ(utils::TimeSpan(), task.StartDelay) << expectedTaskName;
		EXPECT_EQ(utils::TimeSpan(), task.RepeatDelay) << expectedTaskName;
		EXPECT_TRUE(!!task.Callback) << expectedTaskName;
	}
}}
