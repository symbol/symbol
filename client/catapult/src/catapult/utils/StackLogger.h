#pragma once
#include "Logging.h"
#include <chrono>

namespace catapult { namespace utils {

	/// Simple RAII class that logs scope messages.
	class StackLogger {
	public:
		/// Constructs a logger with a message (\a pMessage) and log \a level.
		explicit StackLogger(const char* pMessage, LogLevel level)
				: m_pMessage(pMessage)
				, m_level(level) {
			CATAPULT_LOG_LEVEL(m_level) << "pushing scope '" << m_pMessage << "'";

			// start timer after logging
			m_start = std::chrono::steady_clock::now();
		}

		/// Destructor.
		~StackLogger() {
			// stop timer before logging
			auto elapsedMillis = millis();
			CATAPULT_LOG_LEVEL(m_level) << "popping scope '" << m_pMessage << "' (" << elapsedMillis << "ms)";
		}

	public:
		/// Gets the number of elapsed milliseconds since this logger was created.
		uint64_t millis() const {
			auto elapsedDuration = std::chrono::steady_clock::now() - m_start;
			return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsedDuration).count());
		}

	private:
		const char* m_pMessage;
		LogLevel m_level;
		std::chrono::steady_clock::time_point m_start;
	};
}}
