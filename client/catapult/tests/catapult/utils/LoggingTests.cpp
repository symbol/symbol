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
			CATAPULT_LOG(fatal) << "fatal termination";
			CATAPULT_LOG(error) << "baz error";
		}

		void LogAllLevelsWithExplicitLevelMacro() {
			CATAPULT_LOG_LEVEL(LogLevel::Trace) << "alice trace message";
			CATAPULT_LOG_LEVEL(LogLevel::Info) << "foo info";
			CATAPULT_LOG_LEVEL(LogLevel::Debug) << "bob debug message";
			CATAPULT_LOG_LEVEL(LogLevel::Warning) << "bar warning";
			CATAPULT_LOG_LEVEL(LogLevel::Fatal) << "fatal termination";
			CATAPULT_LOG_LEVEL(LogLevel::Error) << "baz error";
		}

		void LogMessagesWithSubcomponentTag(const char* tag) {
#define CATAPULT_LOG_TAG(LEVEL) \
	CATAPULT_LOG_WITH_LOGGER_LEVEL_TAG( \
		log::global_logger::get(), \
		LEVEL, \
		RawString(tag, strlen(tag)))

			CATAPULT_LOG_TAG(LogLevel::Trace) << "alice trace message";
			CATAPULT_LOG_TAG(LogLevel::Info) << "foo info";
			CATAPULT_LOG_TAG(LogLevel::Error) << "baz error";
#undef CATAPULT_LOG_TAG
		}

		void AddUnfilteredFileLogger(LoggingBootstrapper& bootstrapper) {
			bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), LogFilter(LogLevel::Min));
		}
	}

	TEST(TEST_CLASS, CanWriteLogMessagesWithCatapultLogMacro) {
		test::TempFileGuard logFileGuard(test::Test_Log_Filename);

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
			"<trace> (utils::LoggingTests.cpp@14) alice trace message",
			"<info> (utils::LoggingTests.cpp@15) foo info",
			"<debug> (utils::LoggingTests.cpp@16) bob debug message",
			"<warning> (utils::LoggingTests.cpp@17) bar warning",
			"<fatal> (utils::LoggingTests.cpp@18) fatal termination",
			"<error> (utils::LoggingTests.cpp@19) baz error"
		});
	}

	TEST(TEST_CLASS, CanWriteLogMessagesWithCatapultLogLevelMacro) {
		test::TempFileGuard logFileGuard(test::Test_Log_Filename);

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
			"<trace> (utils::LoggingTests.cpp@23) alice trace message",
			"<info> (utils::LoggingTests.cpp@24) foo info",
			"<debug> (utils::LoggingTests.cpp@25) bob debug message",
			"<warning> (utils::LoggingTests.cpp@26) bar warning",
			"<fatal> (utils::LoggingTests.cpp@27) fatal termination",
			"<error> (utils::LoggingTests.cpp@28) baz error"
		});
	}

	TEST(TEST_CLASS, CanWriteLogMessagesWithCustomComponentTags) {
		test::TempFileGuard logFileGuard(test::Test_Log_Filename);

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
			"<trace> (foo::LoggingTests.cpp@38) alice trace message",
			"<info> (foo::LoggingTests.cpp@39) foo info",
			"<error> (foo::LoggingTests.cpp@40) baz error",
			"<trace> (bar::LoggingTests.cpp@38) alice trace message",
			"<info> (bar::LoggingTests.cpp@39) foo info",
			"<error> (bar::LoggingTests.cpp@40) baz error"
		});
	}

	TEST(TEST_CLASS, CanFilterMessagesBySettingGlobalLevel) {
		test::TempFileGuard logFileGuard(test::Test_Log_Filename);

		{
			// Arrange: add a file logger and filter out some messages
			LoggingBootstrapper bootstrapper;
			bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), LogFilter(LogLevel::Info));

			// Act: log messages
			LogAllLevelsWithDefaultMacro();
		}

		// Assert:
		auto records = test::ParseLogLines(logFileGuard.name());
		test::AssertTimestampsAreIncreasing(records);
		test::AssertNumUniqueThreadIds(records, 1);
		test::AssertMessages(records, {
			"<info> (utils::LoggingTests.cpp@15) foo info",
			"<warning> (utils::LoggingTests.cpp@17) bar warning",
			"<fatal> (utils::LoggingTests.cpp@18) fatal termination",
			"<error> (utils::LoggingTests.cpp@19) baz error"
		});
	}

	TEST(TEST_CLASS, CanFilterMessagesBySettingComponentFilterLevelAboveGlobalLevel) {
		test::TempFileGuard logFileGuard(test::Test_Log_Filename);

		{
			// Arrange: add a file logger and filter out some messages
			LogFilter filter(LogLevel::Info);
			filter.setLevel("foo", LogLevel::Error); // foo messages below error should not appear

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
			"<error> (foo::LoggingTests.cpp@40) baz error",
			"<info> (bar::LoggingTests.cpp@39) foo info",
			"<error> (bar::LoggingTests.cpp@40) baz error"
		});
	}

	TEST(TEST_CLASS, CanFilterMessagesBySettingComponentFilterLevelBelowGlobalLevel) {
		test::TempFileGuard logFileGuard(test::Test_Log_Filename);

		{
			// Arrange: add a file logger and filter out some messages
			LogFilter filter(LogLevel::Info);
			filter.setLevel("foo", LogLevel::Min); // all foo messages should appear

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
			"<trace> (foo::LoggingTests.cpp@38) alice trace message",
			"<info> (foo::LoggingTests.cpp@39) foo info",
			"<error> (foo::LoggingTests.cpp@40) baz error",
			"<info> (bar::LoggingTests.cpp@39) foo info",
			"<error> (bar::LoggingTests.cpp@40) baz error"
		});
	}

	TEST(TEST_CLASS, CanFilterMessagesFromRealComponents) {
		test::TempFileGuard logFileGuard(test::Test_Log_Filename);

		{
			// Arrange: add a file logger and filter out some messages
			LogFilter filter(LogLevel::Trace);
			filter.setLevel("utils", LogLevel::Max); // filter out all non-fatal utils messages

			LoggingBootstrapper bootstrapper;
			bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), filter);

			// Act: use a stack logger to generate some logs
			StackLogger stackLogger("test", LogLevel::Info);

			// - log messages with custom tags
			LogMessagesWithSubcomponentTag("bar");
		}

		// Assert:
		auto records = test::ParseLogLines(logFileGuard.name());
		test::AssertTimestampsAreIncreasing(records);
		test::AssertNumUniqueThreadIds(records, 1);
		test::AssertMessages(records, {
			"<trace> (bar::LoggingTests.cpp@38) alice trace message",
			"<info> (bar::LoggingTests.cpp@39) foo info",
			"<error> (bar::LoggingTests.cpp@40) baz error"
		});
	}

	TEST(TEST_CLASS, CanLogAndFilterMessagesFromMultipleThreads) {
		// Arrange:
		test::TempFileGuard logFileGuard(test::Test_Log_Filename);

		{
			// Arrange: construct the tag for each thread
			std::vector<std::string> idStrings;
			for (auto i = 0u; i < test::GetNumDefaultPoolThreads(); ++i)
				idStrings.push_back(std::to_string(i + 1));

			// - add a file logger and filter out some messages
			LogFilter filter(LogLevel::Info);
			filter.setLevel("io", LogLevel::Max);

			LoggingBootstrapper bootstrapper;
			bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), filter);

			// Act: create a threadpool and write logs from each thread
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
				"<info> (" + subcomponent + "::LoggingTests.cpp@39) foo info",
				"<error> (" + subcomponent + "::LoggingTests.cpp@40) baz error"
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
		test::TempFileGuard logFileGuard(test::Test_Log_Filename);
		test::TempFileGuard logSecondaryFileGuard("logs/CatapultLoggingTests_Secondary0000.txt");

		{
			// Arrange: add two file loggers with different filters
			LogFilter secondaryLogFilter(LogLevel::Warning);
			secondaryLogFilter.setLevel("foo", LogLevel::Min); // all foo messages should appear
			secondaryLogFilter.setLevel("bar", LogLevel::Max); // no non-fatal bar messages should appear

			LoggingBootstrapper bootstrapper;
			bootstrapper.addFileLogger(test::CreateTestFileLoggerOptions(), LogFilter(LogLevel::Info));
			bootstrapper.addFileLogger(FileLoggerOptions("logs", "CatapultLoggingTests_Secondary%4N.txt"), secondaryLogFilter);

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
				"<info> (foo::LoggingTests.cpp@39) foo info",
				"<error> (foo::LoggingTests.cpp@40) baz error",
				"<info> (bar::LoggingTests.cpp@39) foo info",
				"<error> (bar::LoggingTests.cpp@40) baz error"
			});
		}

		// - secondary log is composed of foo messages but not bar messages
		{
			CATAPULT_LOG(debug) << "checking secondary log";
			auto secondaryRecords = test::ParseLogLines(logSecondaryFileGuard.name());
			test::AssertTimestampsAreIncreasing(secondaryRecords);
			test::AssertNumUniqueThreadIds(secondaryRecords, 1);
			test::AssertMessages(secondaryRecords, {
				"<trace> (foo::LoggingTests.cpp@38) alice trace message",
				"<info> (foo::LoggingTests.cpp@39) foo info",
				"<error> (foo::LoggingTests.cpp@40) baz error",
			});
		}
	}
}}
