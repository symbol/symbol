#include "HashCacheDelta.h"

namespace catapult { namespace cache {

	namespace {
		constexpr Timestamp GetPruneTime(uint64_t retentionTimeMs, uint64_t timestampMs) {
			return Timestamp(timestampMs < retentionTimeMs ? 0u : timestampMs - retentionTimeMs);
		}
	}

	void BasicHashCacheDelta::prune(Timestamp timestamp) {
		auto pruneTime = GetPruneTime(m_retentionTime.millis(), timestamp.unwrap());
		m_pruningBoundary = ValueType(pruneTime);
	}
}}
