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
#include <chrono>

namespace catapult { namespace utils {

	/// Simple RAII class that logs scope messages.
	class StackLogger {
	private:
		using Clock = std::chrono::steady_clock;

	public:
		/// Constructs a logger with a \a message and log \a level.
		explicit StackLogger(const char* message, LogLevel level)
				: m_message(message)
				, m_level(level) {
			CATAPULT_LOG_LEVEL(m_level) << "pushing scope '" << m_message << "'";

			// start timer after logging
			m_start = Clock::now();
		}

		/// Destructor.
		~StackLogger() {
			// stop timer before logging
			auto elapsedMillis = millis();
			CATAPULT_LOG_LEVEL(m_level) << "popping scope '" << m_message << "' (" << elapsedMillis << "ms)";
		}

	public:
		/// Gets the number of elapsed milliseconds since this logger was created.
		uint64_t millis() const {
			auto elapsedDuration = Clock::now() - m_start;
			return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsedDuration).count());
		}

	private:
		const char* m_message;
		LogLevel m_level;
		Clock::time_point m_start;
	};
}}
