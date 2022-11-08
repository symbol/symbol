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

#include "ProcessMain.h"
#include "Platform.h"
#include "Signals.h"
#include "catapult/config/CatapultConfiguration.h"
#include "catapult/config/CatapultKeys.h"
#include "catapult/config/ValidateConfiguration.h"
#include "catapult/crypto/OpensslInit.h"
#include "catapult/crypto/OpensslMemory.h"
#include "catapult/io/FileLock.h"
#include "catapult/thread/ThreadInfo.h"
#include "catapult/utils/ExceptionLogging.h"
#include "catapult/utils/Logging.h"
#include "catapult/version/version.h"
#include "catapult/preprocessor.h"
#include <iostream>

namespace catapult { namespace process {

	namespace {
		// region initialization utils

		config::CatapultConfiguration LoadConfiguration(int argc, const char** argv, const std::string& extensionsHost) {
			auto resourcesPath = GetResourcesPath(argc, argv);
			std::cout << "loading resources from " << resourcesPath << std::endl;
			return config::CatapultConfiguration::LoadFromPath(resourcesPath, extensionsHost);
		}

		std::unique_ptr<utils::LogFilter> CreateLogFilter(const config::BasicLoggerConfiguration& config) {
			auto pFilter = std::make_unique<utils::LogFilter>(config.Level);
			for (const auto& pair : config.ComponentLevels)
				pFilter->setLevel(pair.first.c_str(), pair.second);

			return pFilter;
		}

		std::shared_ptr<void> SetupLogging(const std::string& host, const config::LoggingConfiguration& config) {
			auto pBootstrapper = std::make_shared<utils::LoggingBootstrapper>();

			pBootstrapper->addFileLogger(config::GetFileLoggerOptions(config.File), *CreateLogFilter(config.File));

			// log version only to file before initializing console
			std::ostringstream versionStream;
			versionStream << "catapult " << host << std::endl;
			version::WriteVersionInformation(versionStream);
			CATAPULT_LOG(important) << versionStream.str();

			pBootstrapper->addConsoleLogger(config::GetConsoleLoggerOptions(config.Console), *CreateLogFilter(config.Console));
			return PORTABLE_MOVE(pBootstrapper);
		}

		[[noreturn]]
		void TerminateHandler() noexcept {
			// 1. if termination is caused by an exception, log it
			if (std::current_exception()) {
				CATAPULT_LOG(fatal)
						<< std::endl << "thread: " << thread::GetThreadName()
						<< std::endl << UNHANDLED_EXCEPTION_MESSAGE("running local node");
			}

			// 2. flush the log and abort
			utils::CatapultLogFlush();
			std::abort();
		}

		// endregion

		void Run(config::CatapultConfiguration&& config, ProcessOptions processOptions, const CreateProcessHost& createProcessHost) {
			auto catapultKeys = config::CatapultKeys(config.User.CertificateDirectory);

			CATAPULT_LOG(important)
					<< "booting process with keys:"
					<< std::endl << " -   CA " << catapultKeys.caPublicKey()
					<< std::endl << " - NODE " << catapultKeys.nodeKeyPair().publicKey();
			auto pProcessHost = createProcessHost(std::move(config), catapultKeys);

			if (ProcessOptions::Exit_After_Termination_Signal == processOptions)
				WaitForTerminationSignal();

			CATAPULT_LOG(important) << "SHUTTING DOWN PROCESS";
			pProcessHost.reset();
		}
	}

	std::filesystem::path GetResourcesPath(int argc, const char** argv) {
		return std::filesystem::path(argc > 1 ? argv[1] : "..") / "resources";
	}

	int ProcessMain(int argc, const char** argv, const std::string& host, const CreateProcessHost& createProcessHost) {
		return ProcessMain(argc, argv, host, ProcessOptions::Exit_After_Termination_Signal, createProcessHost);
	}

	int ProcessMain(
			int argc,
			const char** argv,
			const std::string& host,
			ProcessOptions processOptions,
			const CreateProcessHost& createProcessHost) {
		std::set_terminate(&TerminateHandler);
		thread::SetThreadName(host + " catapult");
		version::WriteVersionInformation(std::cout);

		// 1. setup OpenSSL
		auto pOpensslContext = crypto::SetupOpensslCryptoFunctions();
		crypto::SetupOpensslMemoryFunctions();

		// 2. load and validate the configuration
		auto config = LoadConfiguration(argc, argv, host);
		ValidateConfiguration(config);

		// 3. initialize logging
		auto pLoggingGuard = SetupLogging(host, config.Logging);

		// 4. check instance
		std::filesystem::path lockFilePath = config.User.DataDirectory;
		lockFilePath /= host + ".lock";
		io::FileLock instanceLock(lockFilePath.generic_string());
		if (!instanceLock.try_lock()) {
			CATAPULT_LOG(fatal) << "could not acquire instance lock " << lockFilePath;
			return -3;
		}

		// 5. platform specific settings
		PlatformSettings();

		// 6. run the server
		Run(std::move(config), processOptions, createProcessHost);
		return 0;
	}
}}
