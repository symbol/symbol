#pragma once
#include "DisruptorBarrier.h"
#include <boost/thread.hpp>
#include <vector>
#include <stdint.h>

namespace catapult { namespace disruptor {

	/// Container for disruptor barriers.
	class DisruptorBarriers {
	public:
		/// Creates \a levelsCount barriers with consecutive levels.
		explicit DisruptorBarriers(size_t levelsCount);

	public:
		/// Returns number of barriers.
		CATAPULT_INLINE size_t size() const {
			return m_barriers.size();
		}

		/// Returns a barrier at a given \a level.
		CATAPULT_INLINE const DisruptorBarrier& operator[](size_t level) const {
			return *m_barriers[level];
		}

		/// Returns a barrier at a given \a level.
		CATAPULT_INLINE DisruptorBarrier& operator[](size_t level) {
			return *m_barriers[level];
		}

	private:
		/// Holds all barriers (barrier level is an index).
		std::vector<std::unique_ptr<DisruptorBarrier>> m_barriers;
	};
}}
