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
#include "tests/test/nodeps/Filesystem.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244) /* possible loss of data */
#endif

#include <boost/date_time.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <map>
#include <string>
#include <vector>

namespace catapult { namespace test {

	/// Creates options for a test file logger.
	utils::FileLoggerOptions CreateTestFileLoggerOptions();

	/// Creates options for a test file logger with log file \a prefix.
	utils::FileLoggerOptions CreateTestFileLoggerOptions(const std::string& prefix);

	/// Uses RAII to delete a test logs directory.
	class TempLogsDirectoryGuard final {
	public:
		/// Guards a default test logs directory.
		TempLogsDirectoryGuard();

		/// Guards a test logs directory with log file \a prefix.
		explicit TempLogsDirectoryGuard(const std::string& prefix);

	public:
		/// Gets the name of the log file with \a id.
		std::string name(size_t id = 0);

	private:
		std::string m_prefix;
		TempDirectoryGuard m_directoryGuard;
	};

	/// Parsed log record.
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
