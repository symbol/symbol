#pragma once
#include "catapult/utils/StackLogger.h"

namespace catapult { namespace test {

	/// Logger for a stress thread.
	class StressThreadLogger {
	public:
		/// Creates a logger for a thread named \a threadName.
		explicit StressThreadLogger(const std::string& threadName)
				: m_threadName(threadName)
				, m_pStackLogger(std::make_unique<utils::StackLogger>(m_threadName.c_str(), utils::LogLevel::Trace))
		{}

	public:
		/// Notifies the logger that iteration \a i of \a max has started.
		void notifyIteration(size_t i, size_t max) {
			if (0 == i % (max / 10))
				CATAPULT_LOG(debug) << m_threadName << " at " << i;
		}

	private:
		std::string m_threadName;
		std::unique_ptr<utils::StackLogger> m_pStackLogger;
	};
}}
