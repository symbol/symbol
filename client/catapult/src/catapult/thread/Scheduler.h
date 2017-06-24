#pragma once
#include "Future.h"
#include "catapult/utils/TimeSpan.h"
#include <functional>
#include <memory>
#include <string>

namespace catapult { namespace thread { class IoServiceThreadPool; } }

namespace catapult { namespace thread {

	/// The result of a task.
	enum class TaskResult {
		/// The task should repeat.
		Continue,

		/// The task should not repeat.
		Break
	};

	using TaskCallback = std::function<thread::future<TaskResult> ()>;

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

	/// A scheduler.
	class Scheduler {
	public:
		virtual ~Scheduler() {}

	public:
		/// The number of tasks that are currently either scheduled or executing.
		virtual uint32_t numScheduledTasks() const = 0;

		/// The number of currently executing task callbacks.
		virtual uint32_t numExecutingTaskCallbacks() const = 0;

	public:
		/// Adds a scheduled task to the scheduler.
		virtual void addTask(const Task& task) = 0;

		/// Shutdowns the scheduler.
		virtual void shutdown() = 0;
	};

	/// Creates a scheduler around the specified thread pool (\a pPool).
	std::shared_ptr<Scheduler> CreateScheduler(const std::shared_ptr<IoServiceThreadPool>& pPool);
}}
