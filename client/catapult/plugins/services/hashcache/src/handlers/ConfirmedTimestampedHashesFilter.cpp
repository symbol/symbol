#include "ConfirmedTimestampedHashesFilter.h"
#include "src/cache/HashCache.h"

namespace catapult { namespace handlers {

	ConfirmedTimestampedHashesFilter CreateConfirmedTimestampedHashesFilter(const cache::HashCache& hashCache) {
		return [&hashCache](const auto& timestampedHashes) -> std::vector<const state::TimestampedHash*> {
			ConfirmedTimestampedHashesFilter::result_type unknownTimestampedHashes;
			auto view = hashCache.createView();
			for (const auto& timestampedHash : timestampedHashes)
				if (!view->contains(timestampedHash))
					unknownTimestampedHashes.push_back(&timestampedHash);

			return unknownTimestampedHashes;
		};
	}
}}
