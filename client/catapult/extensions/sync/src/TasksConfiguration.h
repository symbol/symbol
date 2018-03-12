#pragma once
#include "catapult/utils/TimeSpan.h"
#include <boost/filesystem/path.hpp>
#include <unordered_map>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace sync {

	/// Task configuration settings.
	struct TaskConfiguration {
	public:
		/// The delay until the first execution of the task.
		utils::TimeSpan StartDelay;

		/// The delay until the next execution of the task.
		utils::TimeSpan RepeatDelay;

	private:
		TaskConfiguration() = default;

	public:
		/// Creates an uninitialized task configuration.
		static TaskConfiguration Uninitialized();

	public:
		/// Loads a task configuration from \a bag.
		static TaskConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};

	/// Tasks configuration settings.
	struct TasksConfiguration {
	public:
		/// A map of task names to task configurations.
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
		static TasksConfiguration LoadFromPath(const boost::filesystem::path& resourcesPath);
	};
}}
