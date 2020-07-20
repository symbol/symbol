/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "TasksConfiguration.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace sync {

#define LOAD_PROPERTY(NAME) utils::LoadIniProperty(bag, "", #NAME, config.NAME)

	// region UniformTaskConfiguration

	UniformTaskConfiguration UniformTaskConfiguration::Uninitialized() {
		return UniformTaskConfiguration();
	}

	UniformTaskConfiguration UniformTaskConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		UniformTaskConfiguration config;

		LOAD_PROPERTY(StartDelay);
		LOAD_PROPERTY(RepeatDelay);

		utils::VerifyBagSizeExact(bag, 2);
		return config;
	}

	// endregion

	// region DeceleratingTaskConfiguration

	DeceleratingTaskConfiguration DeceleratingTaskConfiguration::Uninitialized() {
		return DeceleratingTaskConfiguration();
	}

	DeceleratingTaskConfiguration DeceleratingTaskConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		DeceleratingTaskConfiguration config;

		LOAD_PROPERTY(StartDelay);
		LOAD_PROPERTY(MinDelay);
		LOAD_PROPERTY(MaxDelay);
		LOAD_PROPERTY(NumPhaseOneRounds);
		LOAD_PROPERTY(NumTransitionRounds);

		utils::VerifyBagSizeExact(bag, 5);
		return config;
	}

	// endregion

#undef LOAD_PROPERTY

	TasksConfiguration TasksConfiguration::Uninitialized() {
		return TasksConfiguration();
	}

	TasksConfiguration TasksConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		TasksConfiguration config;
		for (const auto& section : bag.sections()) {
			auto taskSectionBag = utils::ExtractSectionAsBag(bag, section.c_str());

			TaskConfiguration taskConfig;
			if (taskSectionBag.contains({ "", "repeatDelay" })) {
				taskConfig.TaskType = TasksConfiguration::TaskType::Uniform;
				taskConfig.Uniform = UniformTaskConfiguration::LoadFromBag(taskSectionBag);
			} else {
				taskConfig.TaskType = TasksConfiguration::TaskType::Decelerating;
				taskConfig.Decelerating = DeceleratingTaskConfiguration::LoadFromBag(taskSectionBag);
			}

			config.Tasks.emplace(section, taskConfig);
		}

		return config;
	}

	TasksConfiguration TasksConfiguration::LoadFromPath(const boost::filesystem::path& resourcesPath) {
		return config::LoadIniConfiguration<TasksConfiguration>(resourcesPath / "config-task.properties");
	}
}}
