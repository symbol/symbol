#pragma once
#include "src/state/TimestampedHash.h"
#include <functional>
#include <vector>

namespace catapult { namespace cache { class HashCache; } }

namespace catapult { namespace handlers {

	/// Prototype for filtering hashes that are contained in the hash cache.
	using ConfirmedTimestampedHashesFilter =
			std::function<std::vector<const state::TimestampedHash*> (const state::TimestampedHashRange&)>;

	/// Creates a filter that returns timestamped hashes not found in the hash cache (\a hashCache).
	ConfirmedTimestampedHashesFilter CreateConfirmedTimestampedHashesFilter(const cache::HashCache& hashCache);
}}
