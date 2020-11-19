/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include <chrono>

namespace catapult { namespace utils {

	/// Stack based timer.
	class StackTimer {
	private:
		using Clock = std::chrono::steady_clock;

	public:
		/// Constructs a stack timer.
		StackTimer() : m_start(Clock::now())
		{}

	public:
		/// Gets the number of elapsed milliseconds since this logger was created.
		uint64_t millis() const {
			auto elapsedDuration = Clock::now() - m_start;
			return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsedDuration).count());
		}

	private:
		Clock::time_point m_start;
	};
}}
