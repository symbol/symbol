#pragma once
#include <utility>

namespace catapult {
	namespace io {
		class InputStream;
		class OutputStream;
	}
}

namespace catapult { namespace cache {

	/// Defines types for map-based cache storage given a cache descriptor.
	template<typename TDescriptor>
	struct MapCacheStorageFromDescriptor {
		/// The cache view type.
		using SourceType = typename TDescriptor::CacheViewType;

		/// The cache delta type.
		using DestinationType = typename TDescriptor::CacheDeltaType;

		/// The cache value type.
		using ValueType = std::pair<typename TDescriptor::KeyType, typename TDescriptor::ValueType>;
	};

	/// Defines types for set-based cache storage given a cache descriptor.
	template<typename TDescriptor>
	struct SetCacheStorageFromDescriptor {
		/// The cache view type.
		using SourceType = typename TDescriptor::CacheViewType;

		/// The cache delta type.
		using DestinationType = typename TDescriptor::CacheDeltaType;

		/// The cache value type.
		using ValueType = typename TDescriptor::ValueType;
	};
}}
