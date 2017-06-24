#ifdef __clang__
// workaround for https://llvm.org/bugs/show_bug.cgi?id=25230
#pragma GCC visibility push(default)
#include <string>
#pragma GCC visibility pop
#endif

#include "LoggingTestUtils.h"
#include "tests/TestHarness.h"
#include <fstream>

namespace catapult { namespace utils {

	FileLoggerOptions CreateTestFileLoggerOptions() {
		return FileLoggerOptions("logs", "CatapultLoggingTests%4N.txt");
	}

	namespace {
		SimpleLogRecord ParseRecord(const std::string& logLine) {
			auto timestampEndIndex = logLine.find(" 0x");
			auto customStartIndex = logLine.find("<");

			SimpleLogRecord record;
			record.Timestamp = boost::posix_time::time_from_string(logLine.substr(0, timestampEndIndex));
			record.ThreadId = logLine.substr(timestampEndIndex + 1, customStartIndex - timestampEndIndex - 3);
			record.Message = logLine.substr(customStartIndex);

			auto tagStartIndex = record.Message.find("(") + 1;
			auto tagEndIndex = record.Message.find("::");
			record.Subcomponent = record.Message.substr(tagStartIndex, tagEndIndex - tagStartIndex);
			return record;
		}
	}

	std::map<std::string, std::vector<SimpleLogRecord>> ParseMultiThreadedLogLines(const std::string& logFilename) {
		std::string logLine;
		std::map<std::string, std::vector<SimpleLogRecord>> records;
		auto logFile = std::fstream(logFilename);
		while (std::getline(logFile, logLine)) {
			auto record = ParseRecord(logLine);
			records[record.ThreadId].push_back(ParseRecord(logLine));
		}

		return records;
	}

	std::vector<SimpleLogRecord> ParseLogLines(const std::string& logFilename) {
		std::string logLine;
		std::vector<SimpleLogRecord> records;
		auto logFile = std::fstream(logFilename);
		while (std::getline(logFile, logLine))
			records.push_back(ParseRecord(logLine));

		return records;
	}

	void AssertTimestampsAreIncreasing(const std::vector<SimpleLogRecord>& records) {
		boost::posix_time::ptime last(boost::posix_time::neg_infin);
		for (const auto& record : records) {
			EXPECT_LE(last, record.Timestamp) << "message: " << record.Message;
			last = record.Timestamp;
		}
	}

	void AssertNumUniqueThreadIds(const std::vector<SimpleLogRecord>& records, size_t numExpectedThreadIds) {
		std::set<std::string> threadIds;
		for (const auto& record : records)
			threadIds.insert(record.ThreadId);

		EXPECT_EQ(numExpectedThreadIds, threadIds.size());
	}

	void AssertMessages(const std::vector<SimpleLogRecord>& records, const std::vector<std::string>& expectedMessages) {
		ASSERT_EQ(expectedMessages.size(), records.size());
		for (auto i = 0u; i < records.size(); ++i)
			EXPECT_EQ(expectedMessages[i], records[i].Message);
	}
}}
