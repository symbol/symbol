#pragma once
#include "catapult/thread/Task.h"

namespace catapult { namespace test {

	/// Asserts that \a task has name \a expectedTaskName and is unscheduled.
	void AssertUnscheduledTask(const thread::Task& task, const std::string& expectedTaskName);
}}
