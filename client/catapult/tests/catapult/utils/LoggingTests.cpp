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
#include "catapult/utils/StackLogger.h"
#include "tests/catapult/utils/test/LoggingTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/thread.hpp>

namespace catapult { namespace utils {

#define TEST_CLASS CatapultLoggingTests

	namespace {
		void LogAllLevelsWithDefaultMacro() {
			CATAPULT_LOG(trace) << "alice trace message";
			CATAPULT_LOG(info) << "foo info";
			CATAPULT_LOG(debug) << "bob debug message";
			CATAPULT_LOG(warning) << "bar warning";
			CATAPULT_LOG(important) << "charlie important message!";
			CATAPULT_LOG(fatal) << "fatal termination";
			CATAPULT_LOG(error) << "baz error";
		}

		void LogAllLevelsWithExplicitLevelMacro() {
			CATAPULT_LOG_LEVEL(LogLevel::trace) << "alice trace message";
			CATAPULT_LOG_LEVEL(LogLevel::info) << "foo info";
			CATAPULT_LOG_LEVEL(LogLevel::debug) << "bob debug message";
			CATAPULT_LOG_LEVEL(LogLevel::warning) << "bar warning";
			CATAPULT_LOG_LEVEL(LogLevel::important) << "charlie important message!";
			CATAPULT_LOG_LEVEL(LogLevel::fatal) << "fatal termination";
			CATAPULT_LOG_LEVEL(LogLevel::error) << "baz error";
		}

		void LogMessagesWithSubcomponentTag(const char* tag) {
#define CATAPULT_LOG_TAG(LEVEL) \
	CATAPULT_LOG_WITH_LOGGER_LEVEL_TAG( \
		log::global_logger::get(), \
		LEVEL, \
		RawString(tag, strlen(tag)))

			CATAPULT_LOG_TAG(LogLevel::trace) << "alice trace message";
			CATAPULT_LOG_TAG(LogLevel::info) << "foo info";
			CATAPULT_LOG_TAG(LogLevel::error) << "baz error";
#undef CATAPULT_LOG_TAG
		}

		void AddUnfilteredFileLogger(LoggingBootstrapper& bootstrapper) {
			bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), LogFilter(LogLevel::min));
		}
	}

	TEST(TEST_CLASS, CanOutputLogLevel) {
		EXPECT_EQ("trace", test::ToString(utils::LogLevel::trace));
		EXPECT_EQ("debug", test::ToString(utils::LogLevel::debug));
		EXPECT_EQ("info", test::ToString(utils::LogLevel::info));
		EXPECT_EQ("important", test::ToString(utils::LogLevel::important));
		EXPECT_EQ("warning", test::ToString(utils::LogLevel::warning));
		EXPECT_EQ("error", test::ToString(utils::LogLevel::error));
		EXPECT_EQ("fatal", test::ToString(utils::LogLevel::fatal));

		EXPECT_EQ("trace", test::ToString(utils::LogLevel::min));
		EXPECT_EQ("fatal", test::ToString(utils::LogLevel::max));

		EXPECT_EQ("fatal", test::ToString(static_cast<utils::LogLevel>(0xFFFF)));
	}

	TEST(TEST_CLASS, CanWriteLogMessagesWithCatapultLogMacro) {
		test::TempLogsDirectoryGuard logFileGuard;

		{
			// Arrange: add a file logger
			LoggingBootstrapper bootstrapper;
			AddUnfilteredFileLogger(bootstrapper);

			// Act: log messages
			LogAllLevelsWithDefaultMacro();
		}

		// Assert:
		auto records = test::ParseLogLines(logFileGuard.name());
		test::AssertTimestampsAreIncreasing(records);
		test::AssertNumUniqueThreadIds(records, 1);
		test::AssertMessages(records, {
			"<trace> (utils::LoggingTests.cpp@34) alice trace message",
			"<info> (utils::LoggingTests.cpp@35) foo info",
			"<debug> (utils::LoggingTests.cpp@36) bob debug message",
			"<warning> (utils::LoggingTests.cpp@37) bar warning",
			"<important> (utils::LoggingTests.cpp@38) charlie important message!",
			"<fatal> (utils::LoggingTests.cpp@39) fatal termination",
			"<error> (utils::LoggingTests.cpp@40) baz error"
		});
	}

	TEST(TEST_CLASS, CanWriteLogMessagesWithCatapultLogLevelMacro) {
		test::TempLogsDirectoryGuard logFileGuard;

		{
			// Arrange: add a file logger
			LoggingBootstrapper bootstrapper;
			AddUnfilteredFileLogger(bootstrapper);

			// Act: log messages
			LogAllLevelsWithExplicitLevelMacro();
		}

		// Assert:
		auto records = test::ParseLogLines(logFileGuard.name());
		test::AssertTimestampsAreIncreasing(records);
		test::AssertNumUniqueThreadIds(records, 1);
		test::AssertMessages(records, {
			"<trace> (utils::LoggingTests.cpp@44) alice trace message",
			"<info> (utils::LoggingTests.cpp@45) foo info",
			"<debug> (utils::LoggingTests.cpp@46) bob debug message",
			"<warning> (utils::LoggingTests.cpp@47) bar warning",
			"<important> (utils::LoggingTests.cpp@48) charlie important message!",
			"<fatal> (utils::LoggingTests.cpp@49) fatal termination",
			"<error> (utils::LoggingTests.cpp@50) baz error"
		});
	}

	TEST(TEST_CLASS, CanWriteLogMessagesWithCustomComponentTags) {
		test::TempLogsDirectoryGuard logFileGuard;

		{
			// Arrange: add a file logger
			LoggingBootstrapper bootstrapper;
			AddUnfilteredFileLogger(bootstrapper);

			// Act: log messages with custom tags
			LogMessagesWithSubcomponentTag("foo");
			LogMessagesWithSubcomponentTag("bar");
		}

		// Assert:
		auto records = test::ParseLogLines(logFileGuard.name());
		test::AssertTimestampsAreIncreasing(records);
		test::AssertNumUniqueThreadIds(records, 1);
		test::AssertMessages(records, {
			"<trace> (foo::LoggingTests.cpp@60) alice trace message",
			"<info> (foo::LoggingTests.cpp@61) foo info",
			"<error> (foo::LoggingTests.cpp@62) baz error",
			"<trace> (bar::LoggingTests.cpp@60) alice trace message",
			"<info> (bar::LoggingTests.cpp@61) foo info",
			"<error> (bar::LoggingTests.cpp@62) baz error"
		});
	}

	TEST(TEST_CLASS, CanFilterMessagesBySettingGlobalLevel) {
		test::TempLogsDirectoryGuard logFileGuard;

		{
			// Arrange: add a file logger and filter out some messages
			LoggingBootstrapper bootstrapper;
			bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), LogFilter(LogLevel::info));

			// Act: log messages
			LogAllLevelsWithDefaultMacro();
		}

		// Assert:
		auto records = test::ParseLogLines(logFileGuard.name());
		test::AssertTimestampsAreIncreasing(records);
		test::AssertNumUniqueThreadIds(records, 1);
		test::AssertMessages(records, {
			"<info> (utils::LoggingTests.cpp@35) foo info",
			"<warning> (utils::LoggingTests.cpp@37) bar warning",
			"<important> (utils::LoggingTests.cpp@38) charlie important message!",
			"<fatal> (utils::LoggingTests.cpp@39) fatal termination",
			"<error> (utils::LoggingTests.cpp@40) baz error"
		});
	}

	TEST(TEST_CLASS, CanFilterMessagesBySettingComponentFilterLevelAboveGlobalLevel) {
		test::TempLogsDirectoryGuard logFileGuard;

		{
			// Arrange: add a file logger and filter out some messages
			LogFilter filter(LogLevel::info);
			filter.setLevel("foo", LogLevel::error); // foo messages below error should not appear

			LoggingBootstrapper bootstrapper;
			bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), filter);

			// Act: log messages with custom tags
			LogMessagesWithSubcomponentTag("foo");
			LogMessagesWithSubcomponentTag("bar");
		}

		// Assert:
		auto records = test::ParseLogLines(logFileGuard.name());
		test::AssertTimestampsAreIncreasing(records);
		test::AssertNumUniqueThreadIds(records, 1);
		test::AssertMessages(records, {
			"<error> (foo::LoggingTests.cpp@62) baz error",
			"<info> (bar::LoggingTests.cpp@61) foo info",
			"<error> (bar::LoggingTests.cpp@62) baz error"
		});
	}

	TEST(TEST_CLASS, CanFilterMessagesBySettingComponentFilterLevelBelowGlobalLevel) {
		test::TempLogsDirectoryGuard logFileGuard;

		{
			// Arrange: add a file logger and filter out some messages
			LogFilter filter(LogLevel::info);
			filter.setLevel("foo", LogLevel::min); // all foo messages should appear

			LoggingBootstrapper bootstrapper;
			bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), filter);

			// Act: log messages with custom tags
			LogMessagesWithSubcomponentTag("foo");
			LogMessagesWithSubcomponentTag("bar");
		}

		// Assert:
		auto records = test::ParseLogLines(logFileGuard.name());
		test::AssertTimestampsAreIncreasing(records);
		test::AssertNumUniqueThreadIds(records, 1);
		test::AssertMessages(records, {
			"<trace> (foo::LoggingTests.cpp@60) alice trace message",
			"<info> (foo::LoggingTests.cpp@61) foo info",
			"<error> (foo::LoggingTests.cpp@62) baz error",
			"<info> (bar::LoggingTests.cpp@61) foo info",
			"<error> (bar::LoggingTests.cpp@62) baz error"
		});
	}

	TEST(TEST_CLASS, CanFilterMessagesFromRealComponents) {
		test::TempLogsDirectoryGuard logFileGuard;

		{
			// Arrange: add a file logger and filter out some messages
			LogFilter filter(LogLevel::trace);
			filter.setLevel("utils", LogLevel::max); // filter out all non-fatal utils messages

			LoggingBootstrapper bootstrapper;
			bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), filter);

			// Act: use a stack logger to generate some logs
			StackLogger stackLogger("test", LogLevel::info);

			// - log messages with custom tags
			LogMessagesWithSubcomponentTag("bar");
		}

		// Assert:
		auto records = test::ParseLogLines(logFileGuard.name());
		test::AssertTimestampsAreIncreasing(records);
		test::AssertNumUniqueThreadIds(records, 1);
		test::AssertMessages(records, {
			"<trace> (bar::LoggingTests.cpp@60) alice trace message",
			"<info> (bar::LoggingTests.cpp@61) foo info",
			"<error> (bar::LoggingTests.cpp@62) baz error"
		});
	}

	TEST(TEST_CLASS, CanLogAndFilterMessagesFromMultipleThreads) {
		// Arrange:
		test::TempLogsDirectoryGuard logFileGuard;

		{
			// Arrange: construct the tag for each thread
			std::vector<std::string> idStrings;
			for (auto i = 0u; i < test::GetNumDefaultPoolThreads(); ++i)
				idStrings.push_back(std::to_string(i + 1));

			// - add a file logger and filter out some messages
			LogFilter filter(LogLevel::info);
			filter.setLevel("io", LogLevel::max);

			LoggingBootstrapper bootstrapper;
			bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), filter);

			// Act: create a thread pool and write logs from each thread
			std::atomic<size_t> numWaitingThreads(0);
			boost::thread_group threads;
			for (auto i = 0u; i < test::GetNumDefaultPoolThreads(); ++i) {
				threads.create_thread([&numWaitingThreads, &idStrings] {
					auto id = ++numWaitingThreads;
					while (test::GetNumDefaultPoolThreads() != numWaitingThreads) {}

					LogMessagesWithSubcomponentTag(idStrings[id - 1].c_str());
				});
			}

			threads.join_all();
		}

		// Assert:
		auto records = test::ParseMultiThreadedLogLines(logFileGuard.name());
		EXPECT_EQ(test::GetNumDefaultPoolThreads(), records.size());

		// - all threads should output the same messages with unique subcomponent tags
		std::set<std::string> subcomponents;
		for (const auto& pair : records) {
			const auto& subcomponent = pair.second.back().Subcomponent;
			test::AssertTimestampsAreIncreasing(pair.second);
			AssertMessages(pair.second, {
				"<info> (" + subcomponent + "::LoggingTests.cpp@61) foo info",
				"<error> (" + subcomponent + "::LoggingTests.cpp@62) baz error"
			});
			subcomponents.insert(subcomponent);
		}

		// - all subcomponents (unique per thread in this test) should be represented
		std::set<std::string> expectedSubcomponents;
		for (auto i = 0u; i < test::GetNumDefaultPoolThreads(); ++i)
			expectedSubcomponents.insert(std::to_string(i + 1));

		EXPECT_EQ(expectedSubcomponents, subcomponents);
	}

	TEST(TEST_CLASS, CanConfigureLoggersWithIndependentFilters) {
		test::TempLogsDirectoryGuard logFileGuard;
		test::TempLogsDirectoryGuard logSecondaryFileGuard("CatapultLoggingTests_Secondary");

		{
			// Arrange: add two file loggers with different filters
			LogFilter secondaryLogFilter(LogLevel::warning);
			secondaryLogFilter.setLevel("foo", LogLevel::min); // all foo messages should appear
			secondaryLogFilter.setLevel("bar", LogLevel::max); // no non-fatal bar messages should appear

			LoggingBootstrapper bootstrapper;
			bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), LogFilter(LogLevel::info));
			bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions("CatapultLoggingTests_Secondary"), secondaryLogFilter);

			// Act: log messages with custom tags
			LogMessagesWithSubcomponentTag("foo");
			LogMessagesWithSubcomponentTag("bar");
		}

		// Assert: primary log is composed of all messages at least informational level
		{
			CATAPULT_LOG(debug) << "checking primary log";
			auto primaryRecords = test::ParseLogLines(logFileGuard.name());
			test::AssertTimestampsAreIncreasing(primaryRecords);
			test::AssertNumUniqueThreadIds(primaryRecords, 1);
			test::AssertMessages(primaryRecords, {
				"<info> (foo::LoggingTests.cpp@61) foo info",
				"<error> (foo::LoggingTests.cpp@62) baz error",
				"<info> (bar::LoggingTests.cpp@61) foo info",
				"<error> (bar::LoggingTests.cpp@62) baz error"
			});
		}

		// - secondary log is composed of foo messages but not bar messages
		{
			CATAPULT_LOG(debug) << "checking secondary log";
			auto secondaryRecords = test::ParseLogLines(logSecondaryFileGuard.name());
			test::AssertTimestampsAreIncreasing(secondaryRecords);
			test::AssertNumUniqueThreadIds(secondaryRecords, 1);
			test::AssertMessages(secondaryRecords, {
				"<trace> (foo::LoggingTests.cpp@60) alice trace message",
				"<info> (foo::LoggingTests.cpp@61) foo info",
				"<error> (foo::LoggingTests.cpp@62) baz error"
			});
		}
	}
}}
