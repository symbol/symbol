#pragma once
#include "UtCache.h"
#include "UtChangeSubscriber.h"

namespace catapult { namespace cache {

	/// Creates an aggregate ut cache that delegates to \a utCache and publishes ut transaction changes to \a pUtChangeSubscriber.
	std::unique_ptr<UtCache> CreateAggregateUtCache(UtCache& utCache, std::unique_ptr<UtChangeSubscriber>&& pUtChangeSubscriber);
}}
