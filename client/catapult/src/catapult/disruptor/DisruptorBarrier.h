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
#include "DisruptorTypes.h"
#include "catapult/utils/Logging.h"
#include <atomic>
#include <stddef.h>
#include <stdint.h>

namespace catapult { namespace disruptor {

	/// DisruptorBarrier represents a consumer barrier (possibly shared by multiple consumers)
	/// at a given level.
	class DisruptorBarrier {
	public:
		/// Creates a barrier given its \a level and position (\a barrierEndPosition).
		DisruptorBarrier(size_t level, PositionType position)
				: m_level(level)
				, m_position(position)
		{}

		/// Advances the barrier.
		inline void advance() {
			++m_position;
		}

		/// Gets the level of the barrier.
		inline size_t level() const {
			return m_level;
		}

		/// Gets the position of the barrier.
		inline PositionType position() const {
			return m_position;
		}

	private:
		const size_t m_level;
		std::atomic<PositionType> m_position;
	};
}}
