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

#include "LoggingConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace config {

	LoggingConfiguration LoggingConfiguration::Uninitialized() {
		return LoggingConfiguration();
	}

	LoggingConfiguration LoggingConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		LoggingConfiguration config;

#define LOAD_CONSOLE_LOGGER_PROPERTY(NAME) utils::LoadIniProperty(bag, "console", #NAME, config.Console.NAME)

		LOAD_CONSOLE_LOGGER_PROPERTY(SinkType);
		LOAD_CONSOLE_LOGGER_PROPERTY(Level);
		LOAD_CONSOLE_LOGGER_PROPERTY(ColorMode);

#undef LOAD_CONSOLE_LOGGER_PROPERTY

#define LOAD_FILE_LOGGER_PROPERTY(NAME) utils::LoadIniProperty(bag, "file", #NAME, config.File.NAME)

		LOAD_FILE_LOGGER_PROPERTY(SinkType);
		LOAD_FILE_LOGGER_PROPERTY(Level);
		LOAD_FILE_LOGGER_PROPERTY(Directory);
		LOAD_FILE_LOGGER_PROPERTY(FilePattern);
		LOAD_FILE_LOGGER_PROPERTY(RotationSize);
		LOAD_FILE_LOGGER_PROPERTY(MaxTotalSize);
		LOAD_FILE_LOGGER_PROPERTY(MinFreeSpace);

#undef LOAD_FILE_LOGGER_PROPERTY

		// load optional properties
		config.Console.ComponentLevels = bag.getAll<utils::LogLevel>("console.component.levels");
		config.File.ComponentLevels = bag.getAll<utils::LogLevel>("file.component.levels");

		utils::VerifyBagSizeExact(bag, 10 + config.Console.ComponentLevels.size() + config.File.ComponentLevels.size());
		return config;
	}

	// region logger configuration -> logger options

	utils::BasicLoggerOptions GetConsoleLoggerOptions(const ConsoleLoggerConfiguration& config) {
		utils::BasicLoggerOptions options;
		options.SinkType = config.SinkType;
		options.ColorMode = config.ColorMode;
		return options;
	}

	utils::FileLoggerOptions GetFileLoggerOptions(const FileLoggerConfiguration& config) {
		utils::FileLoggerOptions options(config.Directory, config.FilePattern);
		options.SinkType = config.SinkType;

		options.RotationSize = config.RotationSize.bytes();
		options.MaxTotalSize = config.MaxTotalSize.bytes();
		options.MinFreeSpace = config.MinFreeSpace.bytes();
		return options;
	}

	// endregion
}}
