#include "catapult/utils/Logging.h"
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
