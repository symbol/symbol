#pragma once
#include "Task.h"
#include <string>

namespace catapult { namespace thread { class IoServiceThreadPool; } }

namespace catapult { namespace thread {

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
