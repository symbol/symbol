#pragma once
#include <stdint.h>

namespace catapult { namespace cache {

	/// Logs cache sizes for a cache with \a name at specified levels of fullness given its \a actual and \a max sizes.
	inline
	void LogSizes(const char* name, size_t actual, uint64_t max) {
		auto logCacheSizeIf = [name, actual](uint64_t desired, const char* description) {
			if (actual != desired)
				return;

			CATAPULT_LOG(warning) << name << " cache is " << description << " (size = " << actual << ")";
		};

		// log if cache is filling up (or full)
		logCacheSizeIf(max / 2, "half full");
		logCacheSizeIf(max, "full");
	}
}}
