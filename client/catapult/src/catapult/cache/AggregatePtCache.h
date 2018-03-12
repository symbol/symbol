#pragma once
#include "PtCache.h"
#include "PtChangeSubscriber.h"

namespace catapult { namespace cache {

	/// Creates an aggregate pt cache that delegates to \a ptCache and publishes pt transaction changes to \a pPtChangeSubscriber.
	std::unique_ptr<PtCache> CreateAggregatePtCache(PtCache& ptCache, std::unique_ptr<PtChangeSubscriber>&& pPtChangeSubscriber);
}}
