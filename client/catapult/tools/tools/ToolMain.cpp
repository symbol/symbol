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
#include "catapult/thread/ThreadInfo.h"
#include "catapult/utils/ExceptionLogging.h"
#include "catapult/version/version.h"
#include "catapult/preprocessor.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

namespace catapult { namespace tools {

	namespace {
		// region initialization utils

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
			config.Console.Level = utils::LogLevel::debug;
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
			return PORTABLE_MOVE(pBootstrapper);
		}

		[[noreturn]]
		void TerminateHandler() noexcept {
			// 1. if termination is caused by an exception, log it
			if (std::current_exception()) {
				CATAPULT_LOG(fatal)
						<< std::endl << "thread: " << thread::GetThreadName()
						<< std::endl << UNHANDLED_EXCEPTION_MESSAGE("running a tool");
			}

			// 2. flush the log and abort
			utils::CatapultLogFlush();
			std::abort();
		}

		// endregion

		// region ParseOptions

		struct ParsedOptions {
			Options ToolOptions;
			bool IsHelpRequest;
			std::string LoggingConfigurationPath;
		};

		void ParseSubOptions(ParsedOptions& options, CommandParser& parser, const std::vector<std::string>& args) {
			namespace po = boost::program_options;

			// set length to 1000 and let terminal handle wrapping
			po::options_description descriptor(parser.name() + " options", 1000, 1000 - 20);
			auto optionsBuilder = descriptor.add_options();

			// add tool-specific options
			OptionsPositional positional;
			parser.prepareOptions(optionsBuilder, positional);

			po::store(po::command_line_parser(args).options(descriptor).run(), options.ToolOptions);

			if (options.IsHelpRequest)
				std::cout << descriptor << std::endl;
		}

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

			// 1. add tool-specific options
			OptionsPositional positional;
			tool.prepareOptions(optionsBuilder, positional);

			// 2. parse all options
			auto parser = po::command_line_parser(argc, argv).options(descriptor).positional(positional);
			if (tool.allowUnregisteredOptions())
				parser.allow_unregistered();

			auto parsed = parser.run();
			po::store(parsed, options.ToolOptions);

			// 3. mark if help was requested and print general help
			options.IsHelpRequest = !!options.ToolOptions.count("help");
			if (options.IsHelpRequest)
				std::cout << descriptor << std::endl;

			// 4. parse sub-command options
			auto* pSubParser = tool.subCommandParser(options.ToolOptions);
			if (pSubParser) {
				auto args = po::collect_unrecognized(parsed.options, po::include_positional);
				// - if command was passed as positional argument, erase it from list of args passed to sub-command parser
				if (positional.max_total_count() > 0) {
					const auto& positionalName = positional.name_for_position(0);
					for (const auto& option : parsed.options) {
						if (positionalName != option.string_key)
							continue;

						if (option.position_key != -1)
							args.erase(args.begin());
					}
				}

				ParseSubOptions(options, *pSubParser, args);
			}

			if (options.IsHelpRequest)
				return;

			// 5. run notifications
			po::notify(options.ToolOptions);
		}

		// endregion
	}

	int ToolMain(int argc, const char** argv, Tool& tool) {
		std::set_terminate(&TerminateHandler);
		thread::SetThreadName("Tool Main");

		std::cout << tool.name() << std::endl;
		version::WriteVersionInformation(std::cout);
		std::cout << std::endl;

		// 1. seed the random number generator
		std::srand(static_cast<unsigned int>(std::time(nullptr)));

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
	}
}}
