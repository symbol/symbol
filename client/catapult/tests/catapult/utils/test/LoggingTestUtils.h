#pragma once
#include "catapult/utils/Logging.h"
#include <boost/date_time.hpp>
#include <map>
#include <string>
#include <vector>

namespace catapult { namespace test {

	/// The name of the test log file.
	constexpr auto Test_Log_Filename = "logs/CatapultLoggingTests0000.txt";

	/// Creates options for a test file logger.
	utils::FileLoggerOptions CreateTestFileLoggerOptions();

	/// A parsed log record.
	struct SimpleLogRecord {
		/// The log timestamp.
		boost::posix_time::ptime Timestamp;

		/// The log thread id.
		std::string ThreadId;

		/// The log message.
		std::string Message;

		/// The log subcomponent.
		std::string Subcomponent;
	};

	/// Parses an unstructured log file (\a logFilename) into simple log records grouped by thread id.
	std::map<std::string, std::vector<SimpleLogRecord>> ParseMultiThreadedLogLines(const std::string& logFilename);

	/// Parses an unstructured log file (\a logFilename) into simple log records.
	std::vector<SimpleLogRecord> ParseLogLines(const std::string& logFilename);

	/// Asserts that \a records have increasing timestamps.
	void AssertTimestampsAreIncreasing(const std::vector<SimpleLogRecord>& records);

	/// Asserts that \a records have \a numExpectedThreadIds unique thread ids.
	void AssertNumUniqueThreadIds(const std::vector<SimpleLogRecord>& records, size_t numExpectedThreadIds);

	/// Asserts that \a records have ordered messages equal to \a expectedMessages.
	void AssertMessages(const std::vector<SimpleLogRecord>& records, const std::vector<std::string>& expectedMessages);
}}
