#pragma once
#include "DisruptorBarrier.h"

namespace catapult { namespace disruptor {

	/// Holds information about a consumer.
	class ConsumerEntry {
	public:
		/// Creates an entry with a \a level and \a position.
		explicit ConsumerEntry(size_t level)
				: m_level(level)
				, m_position(0)
		{}

		/// Advances the position
		PositionType advance() {
			return ++m_position;
		}

	public:
		/// Returns current position (in the circular buffer).
		PositionType position() const {
			return m_position;
		}

		/// Returns consumer level.
		size_t level() const {
			return m_level;
		}

	private:
		const size_t m_level;
		PositionType m_position;
	};
}}
