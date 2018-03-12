#ifdef __clang__
// workaround for https://llvm.org/bugs/show_bug.cgi?id=25230
#pragma GCC visibility push(default)
#include <string>
#pragma GCC visibility pop
#endif

#include "catapult/utils/StackLogger.h"
#include "tests/catapult/utils/test/LoggingTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS StackLoggerTests

	TEST(TEST_CLASS, ElapsedMillisIsInitiallyZero) {
		// Arrange:
		uint64_t elapsedMillis;
		test::RunDeterministicOperation([&elapsedMillis]() {
			StackLogger stackLogger("trace", LogLevel::Trace);

			// Act:
			elapsedMillis = stackLogger.millis();
		});

		// Assert:
		EXPECT_EQ(0u, elapsedMillis);
	}

	TEST(TEST_CLASS, ElapsedMillisIncreasesOverTime) {
		// Arrange:
		StackLogger stackLogger("trace", LogLevel::Trace);

		// Act:
		test::Sleep(5);
		auto elapsedMillis1 = stackLogger.millis();
		test::Sleep(10);
		auto elapsedMillis2 = stackLogger.millis();

		// Assert:
		EXPECT_GT(elapsedMillis1, 0u);
		EXPECT_GT(elapsedMillis2, elapsedMillis1);
	}

	namespace {
		constexpr auto Sleep_Millis = 5u;
		constexpr auto Epsilon_Millis = 1u;

		constexpr auto IsWithinSleepEpsilonRange(uint64_t millis) {
			return Sleep_Millis - Epsilon_Millis <= millis && millis <= Sleep_Millis + Epsilon_Millis;
		}
	}

	TEST(TEST_CLASS, ElapsedMillisCanBeAccessedAtPointInTime) {
		// Arrange: non-deterministic due to sleep
		test::RunNonDeterministicTest("specific elapsed time", []() {
			StackLogger stackLogger("trace", LogLevel::Trace);

			// - wait
			test::Sleep(Sleep_Millis);

			// Act:
			auto elapsedMillis = stackLogger.millis();
			if (!IsWithinSleepEpsilonRange(elapsedMillis)) {
				CATAPULT_LOG(debug) << "elapsedMillis (" << elapsedMillis << ") outside of expected range";
				return false;
			}

			// Assert:
			EXPECT_TRUE(IsWithinSleepEpsilonRange(elapsedMillis)) << "elapsedMillis: " << elapsedMillis;
			return true;
		});
	}

	namespace {
		size_t ParseElapsedMillis(const std::string& message) {
			auto timeEndIndex = message.find("ms");
			auto timeStartIndex = message.rfind("(", timeEndIndex);
			return static_cast<size_t>(std::atoi(message.substr(timeStartIndex + 1, timeEndIndex - timeStartIndex).c_str()));
		}
	}

	TEST(TEST_CLASS, CanLogStackMessages) {
		// Arrange: non-deterministic due to sleep
		test::RunNonDeterministicTest("log stack messages", []() {
			test::TempFileGuard logFileGuard(test::Test_Log_Filename);

			{
				// Arrange: add a file logger
				LoggingBootstrapper bootstrapper;
				bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), LogFilter(LogLevel::Min));

				// Act: log messages by creating and destroying a stack logger
				{
					StackLogger stackLogger("test", LogLevel::Warning);

					// - wait
					test::Sleep(Sleep_Millis);
				}
			}

			// Assert:
			auto records = test::ParseLogLines(logFileGuard.name());
			EXPECT_EQ(2u, records.size());
			if (2u != records.size())
				return true; // test will fail due to previous EXPECT_EQ

			EXPECT_EQ("<warning> (utils::StackLogger.h@17) pushing scope 'test'", records[0].Message);

			auto elapsedMillis = ParseElapsedMillis(records[1].Message);
			if (!IsWithinSleepEpsilonRange(elapsedMillis)) {
				CATAPULT_LOG(debug) << "elapsedMillis (" << elapsedMillis << ") outside of expected range";
				return false;
			}

			auto elapsedMillisString = " (" + std::to_string(elapsedMillis) + "ms)";
			EXPECT_EQ("<warning> (utils::StackLogger.h@27) popping scope 'test'" + elapsedMillisString, records[1].Message);
			return true;
		});
	}
}}
