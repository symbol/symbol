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

#include "catapult/utils/StackLogger.h"
#include "tests/catapult/utils/test/LoggingTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS StackLoggerTests

	// region StackLogger

	namespace {
		constexpr auto Sleep_Millis = 5u;
		constexpr auto Epsilon_Millis = 1u;

		bool IsWithinSleepEpsilonRange(uint64_t millis, uint64_t multiplier = 1) {
			auto expectedMillis = Sleep_Millis * multiplier;
			auto epsilonMillis = Epsilon_Millis * multiplier;
			return expectedMillis - epsilonMillis <= millis && millis <= expectedMillis + epsilonMillis;
		}

		size_t ParseElapsedMillis(const std::string& message) {
			auto timeEndIndex = message.find("ms");
			auto timeStartIndex = message.rfind("(", timeEndIndex);
			auto timeStr = message.substr(timeStartIndex + 1, timeEndIndex - timeStartIndex - 1);
			return static_cast<size_t>(std::atoi(timeStr.c_str()));
		}
	}

	TEST(TEST_CLASS, StackLoggerCanLogStackMessages) {
		// Arrange: non-deterministic due to sleep
		test::RunNonDeterministicTest("log stack messages", []() {
			test::TempLogsDirectoryGuard logFileGuard;

			{
				// Arrange: add a file logger
				LoggingBootstrapper bootstrapper;
				bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), LogFilter(LogLevel::min));

				// Act: log messages by creating and destroying a stack logger
				{
					StackLogger stackLogger("test", LogLevel::warning);

					// - wait
					test::Sleep(Sleep_Millis);
				}
			}

			// Assert:
			auto records = test::ParseLogLines(logFileGuard.name());
			EXPECT_EQ(2u, records.size());
			if (2u != records.size())
				return true; // test will fail due to previous EXPECT_EQ

			EXPECT_EQ("<warning> (utils::StackLogger.h@36) pushing scope 'test'", records[0].Message);

			auto elapsedMillis = ParseElapsedMillis(records[1].Message);
			if (!IsWithinSleepEpsilonRange(elapsedMillis)) {
				CATAPULT_LOG(debug) << "elapsedMillis (" << elapsedMillis << ") outside of expected range";
				return false;
			}

			auto elapsedMillisString = " (" + std::to_string(elapsedMillis) + "ms)";
			EXPECT_EQ("<warning> (utils::StackLogger.h@42) popping scope 'test'" + elapsedMillisString, records[1].Message);
			return true;
		});
	}

	// endregion

	// region SlowOperationLogger

	TEST(TEST_CLASS, SlowOperationLoggerDoesNotLogWhenThresholdIsNotExceeded) {
		// Arrange: non-deterministic due to sleep
		test::RunNonDeterministicTest("log stack messages", []() {
			test::TempLogsDirectoryGuard logFileGuard;

			{
				// Arrange: add a file logger
				LoggingBootstrapper bootstrapper;
				bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), LogFilter(LogLevel::min));

				// Act: no messages should be logged because threshold is greater than wait
				{
					auto threshold = utils::TimeSpan::FromMilliseconds(10 * Sleep_Millis);
					SlowOperationLogger slowOperationLogger("test", LogLevel::warning, threshold);

					// - wait
					test::Sleep(Sleep_Millis);
				}
			}

			// Assert:
			auto records = test::ParseLogLines(logFileGuard.name());
			if (!records.empty()) // can be 0 or 1
				return false;

			EXPECT_TRUE(records.empty());
			return true;
		});
	}

	TEST(TEST_CLASS, SlowOperationLoggerLogsWhenThresholdIsExceeded) {
		// Arrange: non-deterministic due to sleep
		test::RunNonDeterministicTest("log stack messages", []() {
			test::TempLogsDirectoryGuard logFileGuard;

			{
				// Arrange: add a file logger
				LoggingBootstrapper bootstrapper;
				bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), LogFilter(LogLevel::min));

				// Act: messages should be logged because threshold is less than wait
				{
					auto threshold = utils::TimeSpan::FromMilliseconds(Sleep_Millis);
					SlowOperationLogger slowOperationLogger("test", LogLevel::warning, threshold);

					// - wait
					test::Sleep(10 * Sleep_Millis);
				}
			}

			// Assert:
			auto records = test::ParseLogLines(logFileGuard.name());
			if (1u != records.size()) // can be 0 or 1
				return false;

			EXPECT_EQ(1u, records.size());

			auto elapsedMillis = ParseElapsedMillis(records[0].Message);
			if (!IsWithinSleepEpsilonRange(elapsedMillis, 10)) {
				CATAPULT_LOG(debug) << "elapsedMillis (" << elapsedMillis << ") outside of expected range";
				return false;
			}

			auto elapsedMillisString = " (" + std::to_string(elapsedMillis) + "ms)";
			EXPECT_EQ("<warning> (utils::StackLogger.h@76) slow operation detected: 'test'" + elapsedMillisString, records[0].Message);
			return true;
		});
	}

	namespace {
		std::vector<size_t> ParseSubOperationTimesInMillis(const std::string& message) {
			std::vector<size_t> subOperationTimes;

			size_t timeStartIndex = 0;
			for (;;) {
				timeStartIndex = message.find(" + ", timeStartIndex);
				auto timeEndIndex = message.find("ms:", timeStartIndex);
				if (timeStartIndex == std::string::npos)
					break;

				auto timeStr = message.substr(timeStartIndex + 3, timeEndIndex - timeStartIndex - 3);
				subOperationTimes.push_back(static_cast<size_t>(std::atoi(timeStr.c_str())));

				timeStartIndex = timeEndIndex;
			}

			return subOperationTimes;
		}
	}

	TEST(TEST_CLASS, SlowOperationLoggerLogsWhenThresholdIsExceededWithSubOperations) {
		// Arrange: non-deterministic due to sleep
		test::RunNonDeterministicTest("log stack messages", []() {
			test::TempLogsDirectoryGuard logFileGuard;

			{
				// Arrange: add a file logger
				LoggingBootstrapper bootstrapper;
				bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), LogFilter(LogLevel::min));

				// Act: messages should be logged because threshold is less than wait
				{
					auto threshold = utils::TimeSpan::FromMilliseconds(Sleep_Millis);
					SlowOperationLogger slowOperationLogger("test", LogLevel::warning, threshold);

					// - wait with three sub operations
					test::Sleep(Sleep_Millis);
					slowOperationLogger.addSubOperation("zeta");
					test::Sleep(4 * Sleep_Millis);
					slowOperationLogger.addSubOperation("beta");
					test::Sleep(4 * Sleep_Millis);
					slowOperationLogger.addSubOperation("gamma");
					test::Sleep(Sleep_Millis);
				}
			}

			// Assert:
			auto records = test::ParseLogLines(logFileGuard.name());
			if (1u != records.size()) // can be 0 or 1
				return false;

			EXPECT_EQ(1u, records.size());

			auto elapsedMillis = ParseElapsedMillis(records[0].Message);
			if (!IsWithinSleepEpsilonRange(elapsedMillis, 10)) {
				CATAPULT_LOG(debug) << "elapsedMillis (" << elapsedMillis << ") outside of expected range";
				return false;
			}

			auto subOperationTimes = ParseSubOperationTimesInMillis(records[0].Message);
			if (3 != subOperationTimes.size()) {
				// - fail test because message is malformed
				EXPECT_EQ(3u, subOperationTimes.size());
				return true;
			}

			std::ostringstream expectedMessage;
			expectedMessage
					<< "<warning> (utils::StackLogger.h@76) slow operation detected: 'test' (" << elapsedMillis << "ms)"
					<< std::endl << " + " << subOperationTimes[0] << "ms: 'zeta' (" << subOperationTimes[1] - subOperationTimes[0] << "ms)"
					<< std::endl << " + " << subOperationTimes[1] << "ms: 'beta' (" << subOperationTimes[2] - subOperationTimes[1] << "ms)"
					<< std::endl << " + " << subOperationTimes[2] << "ms: 'gamma' (" << elapsedMillis - subOperationTimes[2] << "ms)";
			EXPECT_EQ(expectedMessage.str(), records[0].Message);
			return true;
		});
	}

	// endregion
}}
