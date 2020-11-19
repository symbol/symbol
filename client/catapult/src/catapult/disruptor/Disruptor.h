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
#include "DisruptorBarriers.h"
#include "DisruptorElement.h"
#include "DisruptorTypes.h"
#include "catapult/model/Block.h"
#include "catapult/model/EntityRange.h"
#include "catapult/utils/CircularBuffer.h"
#include "catapult/utils/NonCopyable.h"
#include <vector>

namespace catapult { namespace disruptor {

	/// Disruptor wraps around CircularBuffer for usage within Consumer Dispatcher.
	class Disruptor : utils::NonCopyable {
	public:
		/// Creates disruptor container able to hold \a disruptorSize elements with optional queue logging every
		/// \a elementTraceInterval elements.
		explicit Disruptor(size_t disruptorSize, size_t elementTraceInterval = 1);

	public:
		/// Adds \a input to the underlying container and returns the assigned disruptor element id.
		/// Once the processing of the input is complete, \a processingComplete will be called.
		DisruptorElementId add(ConsumerInput&& input, const ProcessingCompleteFunc& processingComplete);

		/// Sets the skip flag on the element at \a position with \a result.
		void markSkipped(PositionType position, const ConsumerResult& result);

		/// Checks the skip flag on the element at \a position.
		bool isSkipped(PositionType position) const;

		/// Gets the element at given \a position.
		inline DisruptorElement& elementAt(PositionType position) {
			return m_container[position];
		}

		/// Gets the size of the disruptor.
		inline size_t size() const {
			return m_container.size();
		}

		/// Gets the capacity of the disruptor.
		inline size_t capacity() const {
			return m_container.capacity();
		}

		/// Gets the number of total elements added to the disruptor.
		inline uint64_t added() const {
			return m_allElementsCount;
		}

	private:
		size_t m_elementTraceInterval;
		utils::CircularBuffer<DisruptorElement> m_container;
		std::atomic<uint64_t> m_allElementsCount;
	};
}}
