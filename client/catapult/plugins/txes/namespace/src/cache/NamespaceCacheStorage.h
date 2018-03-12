#pragma once
#include "NamespaceCache.h"
#include "catapult/cache/CacheStorageInclude.h"

namespace catapult { namespace cache {

	/// Policy for saving and loading namespace cache data.
	struct NamespaceCacheStorage : public MapCacheStorageFromDescriptor<NamespaceCacheDescriptor> {
		using SourceType = NamespaceCacheView;
		using DestinationType = NamespaceCacheDelta;

		/// Saves \a value to \a output.
		static void Save(const ValueType& value, io::OutputStream& output);

		/// Loads a single value from \a input into \a cacheDelta.
		static void Load(io::InputStream& input, DestinationType& cacheDelta);
	};
}}
