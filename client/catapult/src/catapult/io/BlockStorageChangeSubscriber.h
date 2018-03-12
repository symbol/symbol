#pragma once
#include "BlockChangeSubscriber.h"
#include <memory>

namespace catapult { namespace io { class LightBlockStorage; } }

namespace catapult { namespace io {

	/// Creates a block change subscriber around a block storage (\a pStorage).
	std::unique_ptr<BlockChangeSubscriber> CreateBlockStorageChangeSubscriber(std::unique_ptr<LightBlockStorage>&& pStorage);
}}
