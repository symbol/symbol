#pragma once
#include <stdint.h>

namespace catapult { namespace cache {

	/// Options for customizing the behavior of a memory based cache.
	class MemoryCacheOptions {
	public:
		/// Creates default options.
		constexpr MemoryCacheOptions() : MemoryCacheOptions(0, 0)
		{}

		/// Creates options with custom \a maxResponseSize and \a maxCacheSize.
		constexpr MemoryCacheOptions(uint64_t maxResponseSize, uint64_t maxCacheSize)
				: MaxResponseSize(maxResponseSize)
				, MaxCacheSize(maxCacheSize)
		{}

	public:
		/// The maximum response size.
		uint64_t MaxResponseSize;

		/// The maximum size of the cache.
		uint64_t MaxCacheSize;
	};
}}
