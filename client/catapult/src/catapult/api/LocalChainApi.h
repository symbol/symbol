#pragma once
#include "ChainApi.h"
#include "catapult/functions.h"

namespace catapult { namespace io { class BlockStorageCache; } }

namespace catapult { namespace api {

	/// Creates a chain api around the specified \a storage and chain score supplier (\a chainScoreSupplier)
	/// configured to return at most \a maxHashes hashes per request.
	std::unique_ptr<ChainApi> CreateLocalChainApi(
			const io::BlockStorageCache& storage,
			const model::ChainScoreSupplier& chainScoreSupplier,
			uint32_t maxHashes);
}}
