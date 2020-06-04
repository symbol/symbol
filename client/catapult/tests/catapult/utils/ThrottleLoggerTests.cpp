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

#include "catapult/utils/ThrottleLogger.h"
#include "tests/catapult/utils/test/LoggingTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS ThrottleLoggerTests

	TEST(TEST_CLASS, CounterIsInitiallyZero) {
		// Arrange:
		ThrottleLogger logger(10'000);

		// Act + Assert:
		EXPECT_EQ(0u, logger.counter());
	}

	TEST(TEST_CLASS, FirstLogIsNeverThrottled) {
		// Arrange:
		ThrottleLogger logger(10'000);

		// Act:
		auto isThrottled = logger.isThrottled();

		// Assert:
		EXPECT_FALSE(isThrottled);
		EXPECT_EQ(1u, logger.counter());
	}

	TEST(TEST_CLASS, LogsWithinThrottlePeriodAreThrottled) {
		// Arrange: call isThrottled once to enter a 10s throttle period
		ThrottleLogger logger(10'000);
		logger.isThrottled();

		// Act + Assert:
		for (auto i = 0u; i < 10; ++i) {
			auto isThrottled = logger.isThrottled();
			EXPECT_TRUE(isThrottled) << "at " << i;
		}

		EXPECT_EQ(11u, logger.counter());
	}

#define EXPECT_EQ_RETRY(EXPECTED, ACTUAL) test::ExpectEqualOrRetry((EXPECTED), (ACTUAL), #EXPECTED, #ACTUAL)

	TEST(TEST_CLASS, LogsAreAllowedOutsideOfThrottlePeriod) {
		// Assert: non-deterministic because delay is impacted by sleeps
		test::RunNonDeterministicTest("ThrottleLogger", [](auto i) {
			// Arrange:
			auto timeUnit = test::GetTimeUnitForIteration(i);
			ThrottleLogger logger(2 * timeUnit);

			// Act:
			auto isThrottled = logger.isThrottled();
			EXPECT_FALSE(isThrottled);
			EXPECT_EQ(1u, logger.counter());

			// Assert: after sleeping 0.5x the throttle period, throttling should still be on
			test::Sleep(timeUnit);
			isThrottled = logger.isThrottled();
			EXPECT_EQ(2u, logger.counter());
			if (!EXPECT_EQ_RETRY(true, isThrottled))
				return false;

			// Assert: after sleeping 1.5x the throttle period, throttling should be off
			test::Sleep(2 * timeUnit);
			isThrottled = logger.isThrottled();
			EXPECT_EQ(3u, logger.counter());
			return EXPECT_EQ_RETRY(false, isThrottled);
		});
	}

	TEST(TEST_CLASS, ThrottledMessagesDoNotResetTimer) {
		// Assert: non-deterministic because delay is impacted by sleeps
		test::RunNonDeterministicTest("ThrottleLogger", [](auto i) {
			// Arrange:
			auto timeUnit = test::GetTimeUnitForIteration(i);
			ThrottleLogger logger(4 * timeUnit);

			// Act:
			auto isThrottled = logger.isThrottled();
			EXPECT_FALSE(isThrottled);
			EXPECT_EQ(1u, logger.counter());

			// Assert: after sleeping 0.5x the throttle period, throttling should still be on
			test::Sleep(2 * timeUnit);
			isThrottled = logger.isThrottled();
			EXPECT_EQ(2u, logger.counter());
			if (!EXPECT_EQ_RETRY(true, isThrottled))
				return false;

			// Assert: after sleeping 0.75x the throttle period, throttling should still be on
			test::Sleep(timeUnit);
			isThrottled = logger.isThrottled();
			EXPECT_EQ(3u, logger.counter());
			if (!EXPECT_EQ_RETRY(true, isThrottled))
				return false;

			// Assert: after sleeping 1.25x the throttle period, throttling should be off
			test::Sleep(2 * timeUnit);
			isThrottled = logger.isThrottled();
			EXPECT_EQ(4u, logger.counter());
			return EXPECT_EQ_RETRY(false, isThrottled);
		});
	}

	namespace {
		struct ThrottledMessage {
			static constexpr auto Throttle_Millis = 100u;
			static size_t Num_Logs;

			static void Log(uint32_t value) {
				CATAPULT_LOG_THROTTLE(warning, Throttle_Millis) << "throttled message " << value;
				++Num_Logs;
			}
		};

		size_t ThrottledMessage::Num_Logs = 0;

		std::string CreateExpectedThrottledMessage(uint64_t numLogAttempts, uint32_t value) {
			std::ostringstream out;
			out << "<warning> (utils::ThrottleLoggerTests.cpp@133) [" << numLogAttempts << " log count] throttled message " << value;
			return out.str();
		}
	}

	TEST(TEST_CLASS, CanThrottleViaMacro) {
		// Arrange: non-deterministic due to sleep
		test::RunNonDeterministicTest("log throttled messages", []() {
			auto numBaseLogAttempts = ThrottledMessage::Num_Logs;
			test::TempLogsDirectoryGuard logFileGuard;

			{
				// Arrange: add a file logger
				LoggingBootstrapper bootstrapper;
				bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), LogFilter(LogLevel::min));

				// Act: log messages using a throttle logger and sleeps (same rationale as LogsAreAllowedOutsideOfThrottlePeriod)
				{
					ThrottledMessage::Log(4);
					test::Sleep(ThrottledMessage::Throttle_Millis / 2);
					ThrottledMessage::Log(9);
					test::Sleep(ThrottledMessage::Throttle_Millis);
					ThrottledMessage::Log(16);
				}
			}

			// Assert:
			auto records = test::ParseLogLines(logFileGuard.name());
			if (!EXPECT_EQ_RETRY(2u, records.size())) {
				test::Sleep(4 * ThrottledMessage::Throttle_Millis); // try to clear the throttle
				return false;
			}

			EXPECT_EQ(CreateExpectedThrottledMessage(numBaseLogAttempts + 1, 4), records[0].Message);
			EXPECT_EQ(CreateExpectedThrottledMessage(numBaseLogAttempts + 3, 16), records[1].Message);
			return true;
		});
	}
}}
