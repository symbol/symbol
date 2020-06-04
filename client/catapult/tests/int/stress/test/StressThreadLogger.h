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
#include "catapult/utils/StackLogger.h"

namespace catapult { namespace test {

	/// Logger for a stress thread.
	class StressThreadLogger {
	public:
		/// Creates a logger for a thread named \a threadName.
		explicit StressThreadLogger(const std::string& threadName)
				: m_threadName(threadName)
				, m_pStackLogger(std::make_unique<utils::StackLogger>(m_threadName.c_str(), utils::LogLevel::trace))
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
