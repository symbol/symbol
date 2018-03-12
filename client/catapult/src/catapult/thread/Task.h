#pragma once
#include "Future.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/preprocessor.h"
#include <functional>
#include <string>

namespace catapult { namespace thread {

	/// The result of a task.
	enum class TaskResult {
		/// The task should repeat.
		Continue,

		/// The task should not repeat.
		Break
	};

	/// Task callback that is invoked by the scheduler.
	using TaskCallback = supplier<thread::future<TaskResult>>;

	/// A task that can be dispatched to the scheduler.
	struct Task {
		/// The delay until the first execution of the task.
		utils::TimeSpan StartDelay;

		/// The delay until the next execution of the task.
		utils::TimeSpan RepeatDelay;

		/// The callback associated with the task.
		TaskCallback Callback;

		/// The (optional) friendly name of the task.
		std::string Name;
	};

	/// Creates an unscheduled task with \a name and \a callback.
	CATAPULT_INLINE
	Task CreateNamedTask(const std::string& name, const TaskCallback& callback) {
		Task task;
		task.Name = name;
		task.Callback = callback;
		return task;
	}
}}
