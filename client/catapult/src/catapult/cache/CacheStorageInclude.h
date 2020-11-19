/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include <utility>

namespace catapult {
	namespace io {
		class InputStream;
		class OutputStream;
	}
}

namespace catapult { namespace cache {

	/// Defines types for cache storage given a cache descriptor.
	template<typename TDescriptor>
	struct CacheStorageFromDescriptor {
		/// Cache view type.
		using SourceType = typename TDescriptor::CacheViewType;

		/// Cache delta type.
		using DestinationType = typename TDescriptor::CacheDeltaType;

		/// Cache key type.
		using KeyType = typename TDescriptor::KeyType;

		/// Cache value type.
		using ValueType = typename TDescriptor::ValueType;
	};

	/// Defines cache storage for cache with basic insert remove support.
	template<typename TDescriptor>
	struct CacheStorageForBasicInsertRemoveCache : public CacheStorageFromDescriptor<TDescriptor> {
		/// Loads \a value into \a cacheDelta.
		static void LoadInto(const typename TDescriptor::ValueType& value, typename TDescriptor::CacheDeltaType& cacheDelta) {
			cacheDelta.insert(value);
		}

		/// Purges \a value from \a cacheDelta.
		static void Purge(const typename TDescriptor::ValueType& value, typename TDescriptor::CacheDeltaType& cacheDelta) {
			auto key = TDescriptor::GetKeyFromValue(value);
			if (cacheDelta.contains(key))
				cacheDelta.remove(key);
		}
	};
}}
