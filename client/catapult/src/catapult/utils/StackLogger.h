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
			if (TimeSpan::FromMilliseconds(elapsedMillis) > m_threshold)
				CATAPULT_LOG_LEVEL(m_level) << "slow operation detected: '" << m_message << "' (" << elapsedMillis << "ms)";
		}

	private:
		const char* m_message;
		LogLevel m_level;
		TimeSpan m_threshold;
		StackTimer m_timer;
	};
}}
