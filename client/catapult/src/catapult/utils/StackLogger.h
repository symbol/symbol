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
#include "Logging.h"
#include "StackTimer.h"
#include "TimeSpan.h"
#include <sstream>

namespace catapult { namespace utils {

	/// Simple RAII class that logs scope messages.
	class StackLogger {
	public:
		/// Constructs a logger with \a message and log \a level.
		StackLogger(const char* message, LogLevel level)
				: m_message(message)
				, m_level(level) {
			CATAPULT_LOG_LEVEL(m_level) << "pushing scope '" << m_message << "'";
		}

		/// Destructor.
		~StackLogger() {
			auto elapsedMillis = m_timer.millis();
			CATAPULT_LOG_LEVEL(m_level) << "popping scope '" << m_message << "' (" << elapsedMillis << "ms)";
		}

	private:
		const char* m_message;
		LogLevel m_level;
		StackTimer m_timer;
	};

	/// Simple RAII class that logs slow operation messages.
	class SlowOperationLogger {
	public:
		/// Constructs a logger with \a message and log \a level for messages longer than \a threshold.
		SlowOperationLogger(const char* message, LogLevel level, const TimeSpan& threshold = TimeSpan::FromSeconds(1))
				: m_message(message)
				, m_level(level)
				, m_threshold(threshold)
		{}

		/// Destructor.
		~SlowOperationLogger() {
			auto elapsedMillis = m_timer.millis();
			if (TimeSpan::FromMilliseconds(elapsedMillis) <= m_threshold)
				return;

			std::ostringstream out;
			out << "slow operation detected: '" << m_message << "' (" << elapsedMillis << "ms)";
			for (auto i = 0u; i < m_subOperations.size(); ++i) {
				const auto& pair = m_subOperations[i];
				auto endMillis = (i == m_subOperations.size() - 1) ? elapsedMillis : m_subOperations[i + 1].second;

				out << std::endl << " + " << pair.second << "ms: '" << pair.first << "' (" << endMillis - pair.second << "ms)";
			}

			CATAPULT_LOG_LEVEL(m_level) << out.str();
		}

	public:
		/// Adds a sub operation with \a name for tracking.
		void addSubOperation(const char* name) {
			m_subOperations.emplace_back(name, m_timer.millis());
		}

	private:
		const char* m_message;
		LogLevel m_level;
		TimeSpan m_threshold;
		StackTimer m_timer;
		std::vector<std::pair<const char*, uint64_t>> m_subOperations;
	};
}}
