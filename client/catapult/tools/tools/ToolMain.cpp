#include "ToolMain.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/net/ServerConnector.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/exceptions.h"
#include <boost/exception/diagnostic_information.hpp>
#include <iostream>

namespace catapult { namespace tools {

	namespace {
		std::shared_ptr<void> SetupLogging() {
			utils::BasicLoggerOptions options;
			options.SinkType = utils::LogSinkType::Sync;

			auto pBootstrapper = std::make_shared<utils::LoggingBootstrapper>();
			pBootstrapper->addConsoleLogger(options, utils::LogFilter(utils::LogLevel::Debug));
			return pBootstrapper;
		}
	}

	int ToolMain(int argc, const char** argv, Tool& tool) {
		namespace po = boost::program_options;

		std::srand(static_cast<unsigned int>(std::time(nullptr)));

		std::cout << tool.name() << " Initializing Logging..." << std::endl;
		auto pLoggingGuard = catapult::tools::SetupLogging();

		// set length to 1000, and let terminal handle wrapping
		po::options_description descriptor(tool.name() + " options", 1000, 1000 - 20);
		descriptor.add_options()
				("help,h", "print help message");

		OptionsPositional positional;

		try {
			auto optionsBuilder = descriptor.add_options();
			tool.prepareOptions(optionsBuilder, positional);

			Options options;
			po::store(po::command_line_parser(argc, argv)
					.options(descriptor)
					.positional(positional).run(),
					options);

			if (options.count("help")) {
				std::cout << descriptor << std::endl;
				return 1;
			}

			po::notify(options);
			return tool.run(options);
		} catch (...) {
			CATAPULT_LOG(fatal) << "Unhandled exception while running a tool!" << std::endl
				<< boost::current_exception_diagnostic_information();
			return -1;
		}
	}
}}
