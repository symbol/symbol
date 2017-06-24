#pragma once
#include "DisruptorTypes.h"
#include "catapult/utils/Logging.h"
#include "catapult/preprocessor.h"
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
		CATAPULT_INLINE void advance() {
			++m_position;
		}

		/// Returns level of the barrier.
		CATAPULT_INLINE size_t level() const {
			return m_level;
		}

		/// Returns position of the barrier.
		CATAPULT_INLINE PositionType position() const {
			return m_position;
		}

	private:
		const size_t m_level;
		std::atomic<PositionType> m_position;
	};
}}
