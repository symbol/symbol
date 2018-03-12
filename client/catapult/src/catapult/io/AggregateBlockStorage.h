#pragma once
#include "BlockChangeSubscriber.h"
#include "BlockStorage.h"

namespace catapult { namespace io {

	/// Creates an aggregate block storage that delegates to \a pStorage and publishes block changes to \a pBlockChangeSubscriber.
	std::unique_ptr<BlockStorage> CreateAggregateBlockStorage(
			std::unique_ptr<BlockStorage>&& pStorage,
			std::unique_ptr<BlockChangeSubscriber>&& pBlockChangeSubscriber);
}}
