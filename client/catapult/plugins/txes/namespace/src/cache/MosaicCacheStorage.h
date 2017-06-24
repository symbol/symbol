#pragma once
#include "MosaicCache.h"

namespace catapult {
	namespace io {
		class InputStream;
		class OutputStream;
	}
}

namespace catapult { namespace cache {

	/// Policy for saving and loading mosaic cache data.
	struct MosaicCacheStorage {
		using SourceType = MosaicCacheView;
		using DestinationType = MosaicCacheDelta;
		using ValueType = mosaic_cache_types::mosaic_id_mosaic_history_map::MosaicIdBasedHistoryMap::value_type;

		/// Saves \a value to \a output.
		static void Save(const ValueType& value, io::OutputStream& output);

		/// Loads a single value from \a input into \a cacheDelta.
		static void Load(io::InputStream& input, DestinationType& cacheDelta);
	};
}}
