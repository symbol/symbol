#pragma once
#include "MosaicCacheDelta.h"
#include "MosaicCacheView.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	/// Cache composed of mosaic information.
	using BasicMosaicCache = BasicCache<MosaicCacheDescriptor, MosaicCacheTypes::BaseSetType>;

	/// Synchronized cache composed of mosaic information.
	class MosaicCache : public SynchronizedCache<BasicMosaicCache> {
	public:
		DEFINE_CACHE_CONSTANTS(Mosaic)

	public:
		/// Creates a cache.
		MosaicCache() : SynchronizedCache<BasicMosaicCache>(BasicMosaicCache())
		{}
	};
}}
