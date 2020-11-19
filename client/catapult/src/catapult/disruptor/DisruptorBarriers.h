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
#include <vector>
#include <stdint.h>

namespace catapult { namespace disruptor {

	/// Container for disruptor barriers.
	class DisruptorBarriers {
	public:
		/// Creates \a levelsCount barriers with consecutive levels.
		explicit DisruptorBarriers(size_t levelsCount);

	public:
		/// Gets the number of barriers.
		inline size_t size() const {
			return m_barriers.size();
		}

		/// Gets the (const) barrier at a given \a level.
		inline const DisruptorBarrier& operator[](size_t level) const {
			return *m_barriers[level];
		}

		/// Gets the barrier at a given \a level.
		inline DisruptorBarrier& operator[](size_t level) {
			return *m_barriers[level];
		}

	private:
		/// Holds all barriers (barrier level is an index).
		std::vector<std::unique_ptr<DisruptorBarrier>> m_barriers;
	};
}}
