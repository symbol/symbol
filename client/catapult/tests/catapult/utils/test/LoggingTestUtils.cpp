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

#include "LoggingTestUtils.h"
#include "tests/TestHarness.h"
#include <fstream>

namespace catapult { namespace test {

	utils::FileLoggerOptions CreateTestFileLoggerOptions() {
		return CreateTestFileLoggerOptions("CatapultLoggingTests");
	}

	utils::FileLoggerOptions CreateTestFileLoggerOptions(const std::string& prefix) {
		auto logDirectory = boost::filesystem::path(TempDirectoryGuard::DefaultName()) / "testlogs";
		auto options = utils::FileLoggerOptions(logDirectory.generic_string(), prefix + "%4N.txt");
		options.SinkType = utils::LogSinkType::Sync;
		return options;
	}

	TempLogsDirectoryGuard::TempLogsDirectoryGuard() : TempLogsDirectoryGuard("CatapultLoggingTests")
	{}

	TempLogsDirectoryGuard::TempLogsDirectoryGuard(const std::string& prefix)
			: m_prefix(prefix)
			, m_directoryGuard("testlogs")
	{}

	std::string TempLogsDirectoryGuard::name(size_t id) {
		std::ostringstream logFilename;
		logFilename << m_prefix << std::setfill('0') << std::setw(4) << id << ".txt";
		return (boost::filesystem::path(m_directoryGuard.name()) / logFilename.str()).generic_string();
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
		while (std::getline(logFile, logLine)) {
			// interpret leading space as line continuation
			if (!logLine.empty() && ' ' == logLine[0])
				records.back().Message += "\n" + logLine;
			else
				records.push_back(ParseRecord(logLine));
		}

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
