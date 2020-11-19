/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "Future.h"
#include "catapult/utils/TimeSpan.h"
#include <functional>
#include <string>

namespace catapult { namespace thread {

	/// Result of a task.
	enum class TaskResult {
		/// Task should repeat.
		Continue,

		/// Task should not repeat.
		Break
	};

	/// Task callback that is invoked by the scheduler.
	using TaskCallback = supplier<thread::future<TaskResult>>;

	/// Supplier that generates delays.
	using DelayGenerator = supplier<utils::TimeSpan>;

	/// Task that can be dispatched to a scheduler.
	struct Task {
		/// Delay until the first execution of the task.
		utils::TimeSpan StartDelay;

		/// Generates the delay until the next execution of the task.
		DelayGenerator NextDelay;

		/// Callback associated with the task.
		TaskCallback Callback;

		/// Friendly name of the task (optional).
		std::string Name;
	};

	/// Creates a uniform delay generator that always returns \a delay.
	DelayGenerator CreateUniformDelayGenerator(const utils::TimeSpan& delay);

	/// Creates a three-phase increasing delay generator:
	/// - phase 1: \a minDelay for \a numPhaseOneRounds rounds
	/// - phase 2: delay is linearly increased from \a minDelay to \a maxDelay over \a numTransitionRounds rounds
	/// - phase 3: \a maxDelay for all other rounds
	DelayGenerator CreateIncreasingDelayGenerator(
			const utils::TimeSpan& minDelay,
			uint32_t numPhaseOneRounds,
			const utils::TimeSpan& maxDelay,
			uint32_t numTransitionRounds);

	/// Creates an unscheduled task with \a name and \a callback.
	Task CreateNamedTask(const std::string& name, const TaskCallback& callback);
}}
