#include "DisruptorBarriers.h"
#include "catapult/exceptions.h"

namespace catapult { namespace disruptor {

	DisruptorBarriers::DisruptorBarriers(size_t levelsCount) {
		for (size_t i = 0; i < levelsCount; ++i)
			m_barriers.emplace_back(std::make_unique<DisruptorBarrier>(i, 0));
	}
}}
