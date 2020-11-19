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
#include "catapult/utils/FileSize.h"
#include "catapult/utils/Logging.h"
#include <string>
#include <unordered_map>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace config {

	/// Basic logger configuration settings.
	struct BasicLoggerConfiguration {
		/// Log sink type.
		utils::LogSinkType SinkType;

		/// Log level.
		utils::LogLevel Level;

		/// Custom component log levels.
		std::unordered_map<std::string, utils::LogLevel> ComponentLevels;
	};

	/// Console logger configuration settings.
	struct ConsoleLoggerConfiguration : public BasicLoggerConfiguration {
		/// Console color mode.
		utils::LogColorMode ColorMode;
	};

	/// File logger configuration settings.
	struct FileLoggerConfiguration : public BasicLoggerConfiguration {
		/// Log file directory.
		std::string Directory;

		/// Log file pattern.
		std::string FilePattern;

		/// File rotation size.
		utils::FileSize RotationSize;

		/// Maximum size of all log files.
		utils::FileSize MaxTotalSize;

		/// Minimum size of free disk space in order to create log files.
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
