/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "AccountStateCache.h"
#include "catapult/cache/CacheStorageInclude.h"
#include <vector>

namespace catapult { namespace cache {

	/// Policy for saving and loading account state cache data.
	struct AccountStateCacheStorage : public MapCacheStorageFromDescriptor<AccountStateCacheDescriptor> {
		using LoadStateType = std::vector<uint8_t>;

		/// Saves \a element to \a output.
		static void Save(const StorageType& element, io::OutputStream& output);

		/// Loads a single value from \a input.
		static std::unique_ptr<model::AccountInfo> Load(io::InputStream& input);

		/// Loads a single value from \a input into \a cacheDelta using \a state.
		static void LoadInto(io::InputStream& input, DestinationType& cacheDelta, LoadStateType& state);
	};
}}
