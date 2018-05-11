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

#include "catapult/utils/Logging.h"
#include "catapult/version/version.h"
#include "tests/TestHarness.h"
#include <thread>

#ifdef CATAPULT_DOCKER_TESTS
extern int global_argc;
extern char** global_argv;
int global_argc;
char** global_argv;
#endif

namespace catapult { namespace test {

	uint32_t GetNumDefaultPoolThreads() {
		return 2 * std::thread::hardware_concurrency();
	}

	namespace {
		std::shared_ptr<void> SetupLogging() {
			utils::BasicLoggerOptions options;
			options.SinkType = utils::LogSinkType::Sync;

			auto pBootstrapper = std::make_shared<utils::LoggingBootstrapper>();
			pBootstrapper->addConsoleLogger(options, utils::LogFilter(utils::LogLevel::Debug));
			return pBootstrapper;
		}
	}
}}

int main(int argc, char **argv) {
	catapult::version::WriteVersionInformation(std::cout);
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	std::cout << "Initializing Logging..." << std::endl;
	auto pLoggingGuard = catapult::test::SetupLogging();

	std::cout << "Initializing and Running Tests..." << std::endl;
	::testing::InitGoogleTest(&argc, argv);

#ifdef CATAPULT_DOCKER_TESTS
	global_argc = argc;
	global_argv = argv;
#endif

	return RUN_ALL_TESTS();
}
