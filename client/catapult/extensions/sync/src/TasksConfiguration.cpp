#include "TasksConfiguration.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace sync {

#define LOAD_PROPERTY(NAME) utils::LoadIniProperty(bag, "", #NAME, config.NAME)

	TaskConfiguration TaskConfiguration::Uninitialized() {
		return TaskConfiguration();
	}

	TaskConfiguration TaskConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		TaskConfiguration config;

		LOAD_PROPERTY(StartDelay);
		LOAD_PROPERTY(RepeatDelay);

		utils::VerifyBagSizeLte(bag, 2);
		return config;
	}

#undef LOAD_PROPERTY

	TasksConfiguration TasksConfiguration::Uninitialized() {
		return TasksConfiguration();
	}

	TasksConfiguration TasksConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		TasksConfiguration config;
		for (const auto& section : bag.sections())
			config.Tasks.emplace(section, TaskConfiguration::LoadFromBag(utils::ExtractSectionAsBag(bag, section.c_str())));

		return config;
	}

	TasksConfiguration TasksConfiguration::LoadFromPath(const boost::filesystem::path& resourcesPath) {
		return config::LoadIniConfiguration<TasksConfiguration>(resourcesPath / "config-task.properties");
	}
}}
