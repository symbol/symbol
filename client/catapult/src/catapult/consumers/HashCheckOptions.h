#pragma once
#include <stdint.h>

namespace catapult { namespace consumers {

	/// Options for configuring the hash check consumer.
	struct HashCheckOptions {
	public:
		/// Creates default options.
		constexpr HashCheckOptions() : HashCheckOptions(0, 0, 0)
		{}

		/// Creates options with custom \a cacheDuration, \a pruneInterval and \a maxCacheSize.
		constexpr HashCheckOptions(uint64_t cacheDuration, uint64_t pruneInterval, uint64_t maxCacheSize)
				: CacheDuration(cacheDuration)
				, PruneInterval(pruneInterval)
				, MaxCacheSize(maxCacheSize)
		{}

	public:
		/// The amount of time a hash should be cached.
		uint64_t CacheDuration;

		/// The minimum amount of time between cache pruning.
		uint64_t PruneInterval;

		/// The maximum size of the cache.
		uint64_t MaxCacheSize;
	};
}}
