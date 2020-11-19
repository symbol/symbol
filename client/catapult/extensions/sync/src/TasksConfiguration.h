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
#include "catapult/utils/TimeSpan.h"
#include <filesystem>
#include <unordered_map>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace sync {

	// region UniformTaskConfiguration

	/// Uniform task configuration settings.
	struct UniformTaskConfiguration {
	public:
		/// Delay until the first execution of the task.
		utils::TimeSpan StartDelay;

		/// Delay until subsequent executions of the task.
		utils::TimeSpan RepeatDelay;

	private:
		UniformTaskConfiguration() = default;

	public:
		/// Creates an uninitialized task configuration.
		static UniformTaskConfiguration Uninitialized();

	public:
		/// Loads a task configuration from \a bag.
		static UniformTaskConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};

	// endregion

	// region DeceleratingTaskConfiguration

	/// Decelerating task configuration settings.
	struct DeceleratingTaskConfiguration {
	public:
		/// Delay until the first execution of the task.
		utils::TimeSpan StartDelay;

		/// Minimum delay between task executions.
		utils::TimeSpan MinDelay;

		/// Maximum delay between task executions.
		utils::TimeSpan MaxDelay;

		/// Number of rounds before deceleration starts.
		uint32_t NumPhaseOneRounds;

		/// Number of transition rounds from minimum to maximum delay.
		uint32_t NumTransitionRounds;

	private:
		DeceleratingTaskConfiguration() = default;

	public:
		/// Creates an uninitialized task configuration.
		static DeceleratingTaskConfiguration Uninitialized();

	public:
		/// Loads a task configuration from \a bag.
		static DeceleratingTaskConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};

	// endregion

	/// Tasks configuration settings.
	struct TasksConfiguration {
	public:
		/// Supported task types.
		enum class TaskType {
			/// Task that is scheduled with constant intervals.
			Uniform,

			/// Task that is scheduled with increasing intervals.
			Decelerating
		};

		/// Task configuration that is a union of all supported task configurations.
		struct TaskConfiguration {
		public:
			/// Type of task.
			TasksConfiguration::TaskType TaskType;

			union {
				/// Configuration when task type is uniform.
				UniformTaskConfiguration Uniform;

				/// Configuration when task type is decelerating.
				DeceleratingTaskConfiguration Decelerating;
			};

		public:
			/// Creates an empty task configuration.
			TaskConfiguration() : Uniform(UniformTaskConfiguration::Uninitialized())
			{}
		};

	public:
		/// Map of task names to task configurations.
		std::unordered_map<std::string, TaskConfiguration> Tasks;

	private:
		TasksConfiguration() = default;

	public:
		/// Creates an uninitialized tasks configuration.
		static TasksConfiguration Uninitialized();

	public:
		/// Loads a tasks configuration from \a bag.
		static TasksConfiguration LoadFromBag(const utils::ConfigurationBag& bag);

		/// Loads a tasks configuration from \a resourcesPath.
		static TasksConfiguration LoadFromPath(const std::filesystem::path& resourcesPath);
	};
}}
