#include "HashCacheDelta.h"

namespace catapult { namespace cache {

	BasicHashCacheDelta::BasicHashCacheDelta(
			const HashCacheTypes::BaseSetDeltaPointerType& pHashes,
			const HashCacheTypes::Options& options)
			: SizeMixin<HashCacheTypes::BaseSetDeltaType>(*pHashes)
			, ContainsMixin<HashCacheTypes::BaseSetDeltaType, HashCacheDescriptor>(*pHashes)
			, BasicInsertRemoveMixin<HashCacheTypes::BaseSetDeltaType, HashCacheDescriptor>(*pHashes)
			, m_pOrderedDelta(pHashes)
			, m_retentionTime(options.RetentionTime)
	{}

	utils::TimeSpan BasicHashCacheDelta::retentionTime() const {
		return m_retentionTime;
	}

	deltaset::PruningBoundary<BasicHashCacheDelta::ValueType> BasicHashCacheDelta::pruningBoundary() const {
		return m_pruningBoundary;
	}

	void BasicHashCacheDelta::prune(Timestamp timestamp) {
		auto pruneTime = SubtractNonNegative(timestamp, m_retentionTime);
		m_pruningBoundary = ValueType(pruneTime);
	}
}}
