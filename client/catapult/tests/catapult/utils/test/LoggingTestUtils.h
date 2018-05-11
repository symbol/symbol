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

#pragma once
#include "catapult/utils/Logging.h"
#include <boost/date_time.hpp>
#include <map>
#include <string>
#include <vector>

namespace catapult { namespace test {

	/// Name of the test log file.
	constexpr auto Test_Log_Filename = "logs/CatapultLoggingTests0000.txt";

	/// Creates options for a test file logger.
	utils::FileLoggerOptions CreateTestFileLoggerOptions();

	/// A parsed log record.
	struct SimpleLogRecord {
		/// Log timestamp.
		boost::posix_time::ptime Timestamp;

		/// Log thread id.
		std::string ThreadId;

		/// Log message.
		std::string Message;

		/// Log subcomponent.
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
