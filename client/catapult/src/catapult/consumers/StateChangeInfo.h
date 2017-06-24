#pragma once
#include "catapult/types.h"

namespace catapult {
	namespace cache { class CatapultCacheDelta; }
	namespace model { class ChainScore; }
}

namespace catapult { namespace consumers {

	/// State change information.
	struct StateChangeInfo {
	public:
		/// Creates a new state change info around \a cacheDelta, \a scoreDelta and \a height.
		StateChangeInfo(const cache::CatapultCacheDelta& cacheDelta, const model::ChainScore& scoreDelta, Height height)
				: CacheDelta(cacheDelta)
				, ScoreDelta(scoreDelta)
				, Height(height)
		{}

	public:
		/// The (uncommitted) cache delta.
		const cache::CatapultCacheDelta& CacheDelta;

		/// The chain score delta.
		const model::ChainScore& ScoreDelta;

		/// The new chain height.
		const catapult::Height Height;
	};
}}
