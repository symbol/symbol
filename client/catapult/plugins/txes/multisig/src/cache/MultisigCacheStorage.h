#pragma once
#include "MultisigCache.h"

namespace catapult {
	namespace io {
		class InputStream;
		class OutputStream;
	}
}

namespace catapult { namespace cache {

	/// Policy for saving and loading multisig cache data.
	struct MultisigCacheStorage {
		using SourceType = MultisigCacheView;
		using DestinationType = MultisigCacheDelta;
		using ValueType = multisig_cache_types::account_multisig_entries_map::PublicKeyBasedMultisigEntriesMap::value_type;

		/// Saves \a value to \a output.
		static void Save(const ValueType& value, io::OutputStream& output);

		/// Loads a single value from \a input into \a cacheDelta.
		static void Load(io::InputStream& input, DestinationType& cacheDelta);
	};
}}
