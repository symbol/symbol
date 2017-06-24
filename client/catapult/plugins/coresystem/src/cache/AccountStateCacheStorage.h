#pragma once
#include "AccountStateCache.h"
#include <vector>

namespace catapult {
	namespace io {
		class InputStream;
		class OutputStream;
	}
}

namespace catapult { namespace cache {

	/// Policy for saving and loading account state cache data.
	struct AccountStateCacheStorage {
		using SourceType = AccountStateCacheView;
		using DestinationType = AccountStateCacheDelta;
		using ValueType = account_state_cache_types::address_account_state_map::AddressBasedMap::value_type;
		using LoadStateType = std::vector<uint8_t>;

		/// Saves \a value to \a output.
		static void Save(const ValueType& value, io::OutputStream& output);

		/// Loads a single value from \a input into \a cacheDelta using \a state.
		static void Load(io::InputStream& input, DestinationType& cacheDelta, LoadStateType& state);
	};
}}
