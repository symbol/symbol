#pragma once
#include "catapult/utils/FileSize.h"
#include "catapult/utils/Logging.h"
#include <string>
#include <unordered_map>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace config {

	/// Basic logger configuration settings.
	struct BasicLoggerConfiguration {
		/// The log sink type.
		utils::LogSinkType SinkType;

		/// The log level.
		utils::LogLevel Level;

		/// Custom component log levels.
		std::unordered_map<std::string, utils::LogLevel> ComponentLevels;
	};

	/// Console logger configuration settings.
	struct ConsoleLoggerConfiguration : public BasicLoggerConfiguration {
		/// The console color mode.
		utils::LogColorMode ColorMode;
	};

	/// File logger configuration settings.
	struct FileLoggerConfiguration : public BasicLoggerConfiguration {
		/// The log file directory.
		std::string Directory;

		/// The log file pattern.
		std::string FilePattern;

		/// The file rotation size.
		utils::FileSize RotationSize;

		/// The maximum size of all log files.
		utils::FileSize MaxTotalSize;

		/// The minimum size of free disk space in order to create log files.
		utils::FileSize MinFreeSpace;
	};

	/// Logging configuration settings.
	struct LoggingConfiguration {
	public:
		/// Console logger settings.
		ConsoleLoggerConfiguration Console;

		/// File logger settings.
		FileLoggerConfiguration File;

	private:
		LoggingConfiguration() = default;

	public:
		/// Creates an uninitialized logging configuration.
		static LoggingConfiguration Uninitialized();

	public:
		/// Loads a logging configuration from \a bag.
		static LoggingConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};

	/// Maps console logger configuration (\a config) to console logger options.
	utils::BasicLoggerOptions GetConsoleLoggerOptions(const ConsoleLoggerConfiguration& config);

	/// Maps file logger configuration (\a config) to file logger options.
	utils::FileLoggerOptions GetFileLoggerOptions(const FileLoggerConfiguration& config);
}}
