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

		utils::VerifyBagSizeLte(bag, 10 + config.Console.ComponentLevels.size() + config.File.ComponentLevels.size());
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
