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

#include "Task.h"
#include "catapult/exceptions.h"

namespace catapult { namespace thread {

	DelayGenerator CreateUniformDelayGenerator(const utils::TimeSpan& delay) {
		return [delay]() { return delay; };
	}

	DelayGenerator CreateIncreasingDelayGenerator(
			const utils::TimeSpan& minDelay,
			uint32_t numPhaseOneRounds,
			const utils::TimeSpan& maxDelay,
			uint32_t numTransitionRounds) {
		if (minDelay >= maxDelay)
			CATAPULT_THROW_INVALID_ARGUMENT_2("minDelay must be less than maxDelay", minDelay, maxDelay);

		auto counter = 0u;
		return [counter, minDelay, numPhaseOneRounds, maxDelay, numTransitionRounds]() mutable {
			auto round = ++counter;
			if (round <= numPhaseOneRounds)
				return minDelay;

			if (round >= numPhaseOneRounds + numTransitionRounds)
				return maxDelay;

			auto increment = (maxDelay.millis() - minDelay.millis()) * (round - numPhaseOneRounds) / numTransitionRounds;
			return utils::TimeSpan::FromMilliseconds(minDelay.millis() + increment);
		};
	}

	Task CreateNamedTask(const std::string& name, const TaskCallback& callback) {
		Task task;
		task.Name = name;
		task.Callback = callback;
		return task;
	}
}}
