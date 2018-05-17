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

#include "ToolMain.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/config/LoggingConfiguration.h"
#include "catapult/utils/ExceptionLogging.h"
#include "catapult/version/version.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

namespace catapult { namespace tools {

	namespace {
		config::LoggingConfiguration LoadLoggingConfiguration(const std::string& userLoggingConfigurationPath) {
			// if the user has provided a path, try that first
			std::vector<boost::filesystem::path> loggingConfigurationPaths;
			if (!userLoggingConfigurationPath.empty())
				loggingConfigurationPaths.emplace_back(userLoggingConfigurationPath);

			// fallback to searching in some default locations
			constexpr auto Default_Logging_Configuration_Filename = "config-logging.properties";
			loggingConfigurationPaths.emplace_back(Default_Logging_Configuration_Filename);
			loggingConfigurationPaths.emplace_back(boost::filesystem::path("..") / "resources" / Default_Logging_Configuration_Filename);

			for (const auto& loggingConfigurationPath : loggingConfigurationPaths) {
				if (boost::filesystem::exists(loggingConfigurationPath))
					return config::LoadIniConfiguration<config::LoggingConfiguration>(loggingConfigurationPath);
			}

			auto config = config::LoggingConfiguration::Uninitialized();
			config.Console.SinkType = utils::LogSinkType::Sync;
			config.Console.Level = utils::LogLevel::Debug;
			return config;
		}

		std::unique_ptr<utils::LogFilter> CreateLogFilter(const config::BasicLoggerConfiguration& config) {
			auto pFilter = std::make_unique<utils::LogFilter>(config.Level);
			for (const auto& pair : config.ComponentLevels)
				pFilter->setLevel(pair.first.c_str(), pair.second);

			return pFilter;
		}

		std::shared_ptr<void> SetupLogging(const config::LoggingConfiguration& config) {
			auto pBootstrapper = std::make_shared<utils::LoggingBootstrapper>();
			pBootstrapper->addConsoleLogger(config::GetConsoleLoggerOptions(config.Console), *CreateLogFilter(config.Console));
			return pBootstrapper;
		}

		struct ParsedOptions {
			Options ToolOptions;
			bool IsHelpRequest;
			std::string LoggingConfigurationPath;
		};

		void ParseOptions(ParsedOptions& options, Tool& tool, int argc, const char** argv) {
			namespace po = boost::program_options;

			// set length to 1000 and let terminal handle wrapping
			po::options_description descriptor(tool.name() + " options", 1000, 1000 - 20);
			auto optionsBuilder = descriptor.add_options();

			// add options common for all tools
			optionsBuilder("help,h", "print help message");
			optionsBuilder("loggingConfigurationPath,l",
						OptionsValue<std::string>(options.LoggingConfigurationPath),
						"the path to the logging configuration file");

			// add tool-specific options
			OptionsPositional positional;
			tool.prepareOptions(optionsBuilder, positional);

			// parse all options and handle help requests
			po::store(po::command_line_parser(argc, argv).options(descriptor).positional(positional).run(), options.ToolOptions);
			options.IsHelpRequest = !!options.ToolOptions.count("help");
			if (options.IsHelpRequest) {
				std::cout << descriptor << std::endl;
				return;
			}

			po::notify(options.ToolOptions);
		}
	}

	int ToolMain(int argc, const char** argv, Tool& tool) {
		std::cout << tool.name() << std::endl;
		version::WriteVersionInformation(std::cout);
		std::cout << std::endl;

		// 1. seed the random number generator
		std::srand(static_cast<unsigned int>(std::time(nullptr)));

		try {
			// 2. parse all options
			ParsedOptions options;
			ParseOptions(options, tool, argc, argv);

			// 3. bypass the tool if help was requested
			if (options.IsHelpRequest)
				return 1;

			// 4. initialize logging
			std::cout << tool.name() << " Initializing Logging..." << std::endl;
			auto pLoggingGuard = catapult::tools::SetupLogging(LoadLoggingConfiguration(options.LoggingConfigurationPath));
			std::cout << std::endl;

			// 5. run the tool
			return tool.run(options.ToolOptions);
		} catch (...) {
			CATAPULT_LOG(fatal) << UNHANDLED_EXCEPTION_MESSAGE("running a tool");
			return -1;
		}
	}
}}
