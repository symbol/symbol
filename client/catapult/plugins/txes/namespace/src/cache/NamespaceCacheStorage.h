#pragma once
#include "NamespaceCache.h"

namespace catapult {
	namespace io {
		class InputStream;
		class OutputStream;
	}
}

namespace catapult { namespace cache {

	/// Policy for saving and loading namespace cache data.
	struct NamespaceCacheStorage {
		using SourceType = NamespaceCacheView;
		using DestinationType = NamespaceCacheDelta;
		using ValueType = namespace_cache_types::namespace_id_root_namespace_history_map::NamespaceIdBasedHistoryMap::value_type;

		/// Saves \a value to \a output.
		static void Save(const ValueType& value, io::OutputStream& output);

		/// Loads a single value from \a input into \a cacheDelta.
		static void Load(io::InputStream& input, DestinationType& cacheDelta);
	};
}}
