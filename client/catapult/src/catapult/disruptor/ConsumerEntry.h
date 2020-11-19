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
#include "DisruptorBarrier.h"

namespace catapult { namespace disruptor {

	/// Holds information about a consumer.
	class ConsumerEntry {
	public:
		/// Creates an entry with \a level and \a position.
		explicit ConsumerEntry(size_t level)
				: m_level(level)
				, m_position(0)
		{}

		/// Advances the position
		PositionType advance() {
			return ++m_position;
		}

	public:
		/// Gets the current position (in the circular buffer).
		PositionType position() const {
			return m_position;
		}

		/// Gets the consumer level.
		size_t level() const {
			return m_level;
		}

	private:
		const size_t m_level;
		PositionType m_position;
	};
}}
