#pragma once
#include "DisruptorBarriers.h"
#include "DisruptorElement.h"
#include "catapult/model/Block.h"
#include "catapult/model/EntityRange.h"
#include "catapult/utils/CircularBuffer.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/utils/SpinLock.h"
#include <mutex>
#include <vector>

namespace catapult { namespace disruptor {

	/// Disruptor wraps around CircularBuffer for usage within Consumer Dispatcher.
	class Disruptor : utils::NonCopyable {
	public:
		/// Creates disruptor container able to hold \a disruptorSize elements with optional queue logging every
		/// \a elementTraceInterval elements.
		explicit Disruptor(size_t disruptorSize, size_t elementTraceInterval = 1);

	public:
		/// Adds an \a input to the underlying container and returns the assigned disruptor element id.
		/// Once the processing of the input is complete, \a processingComplete will be called.
		DisruptorElementId add(ConsumerInput&& input, const ProcessingCompleteFunc& processingComplete);

		/// Sets skip flag on the element at \a position with \a code.
		void markSkipped(PositionType position, CompletionCode code);

		/// Checks skip flag on the element at \a position.
		bool isSkipped(PositionType position) const;

		/// Gets element at given position.
		CATAPULT_INLINE DisruptorElement& elementAt(PositionType position) {
			return m_container[position];
		}

		/// Gets the size of the disruptor.
		CATAPULT_INLINE size_t size() const {
			return m_container.size();
		}

		/// Gets the capacity of the disruptor.
		CATAPULT_INLINE size_t capacity() const {
			return m_container.capacity();
		}

		/// Gets the number of total elements added to the disruptor.
		CATAPULT_INLINE uint64_t added() const {
			return m_allElementsCount;
		}

	private:
		using SpinLockGuard = std::lock_guard<utils::SpinLock>;
		size_t m_elementTraceInterval;
		utils::SpinLock m_containerSpinLock;
		utils::CircularBuffer<DisruptorElement> m_container;
		std::atomic<uint64_t> m_allElementsCount;
	};
}}
