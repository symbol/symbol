#include "ServerMain.h"
#include "Signals.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/config/ValidateConfiguration.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/io/FileLock.h"
#include "catapult/utils/Logging.h"
#include <boost/exception/diagnostic_information.hpp>
#include <iostream>

namespace catapult { namespace server {

	namespace {
		config::LocalNodeConfiguration LoadConfiguration(int argc, const char** argv) {
			auto resourcesPath = GetResourcesPath(argc, argv);
			std::cout << "loading resources from " << resourcesPath << std::endl;
			return config::LocalNodeConfiguration::LoadFromPath(resourcesPath);
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
			pBootstrapper->addFileLogger(config::GetFileLoggerOptions(config.File), *CreateLogFilter(config.File));
			return pBootstrapper;
		}

		void Run(config::LocalNodeConfiguration&& config, const CreateLocalNodeFunc& createLocalNode) {
			auto keyPair = crypto::KeyPair::FromString(config.User.BootKey);

			CATAPULT_LOG(info) << "booting local node with public key " << crypto::FormatKey(keyPair.publicKey());
			auto pLocalNode = createLocalNode(std::move(config), keyPair);
			WaitForTerminationSignal();

			CATAPULT_LOG(info) << "shutting down local node";
			pLocalNode.reset();
		}

#define UNHANDLED_EXCEPTION_MESSAGE(ACTION) \
	"Unhandled exception while " << ACTION << "!" << std::endl << boost::current_exception_diagnostic_information();
	}

	boost::filesystem::path GetResourcesPath(int argc, const char** argv) {
		return boost::filesystem::path(argc > 1 ? argv[1] : "..") / "resources";
	}

	int ServerMain(int argc, const char** argv, const CreateLocalNodeFunc& createLocalNode) {
		try {
			// 1. load and validate the configuration
			auto config = LoadConfiguration(argc, argv);
			ValidateConfiguration(config);

			// 2. initialize logging
			auto pLoggingGuard = SetupLogging(config.Logging);

			// 3. check instance
			boost::filesystem::path lockFilePath = config.User.DataDirectory;
			lockFilePath /= "file.lock";
			io::FileLock instanceLock(lockFilePath.string());
			if (!instanceLock.try_lock()) {
				CATAPULT_LOG(fatal) << "could not acquire instance lock " << lockFilePath;
				return -3;
			}

			// 4. run the server
			try {
				Run(std::move(config), createLocalNode);
				return 0;
			} catch (...) {
				CATAPULT_LOG(fatal) << UNHANDLED_EXCEPTION_MESSAGE("running local node");
				return -2;
			}
		} catch (...) {
			std::cerr << UNHANDLED_EXCEPTION_MESSAGE("loading configuration");
			return -1;
		}
	}
}}
