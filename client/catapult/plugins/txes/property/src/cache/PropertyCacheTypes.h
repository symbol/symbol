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
#include "src/state/AccountProperties.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/cache/SingleSetCacheTypesAdapter.h"
#include "catapult/utils/Hashers.h"

namespace catapult {
	namespace cache {
		struct AccountPropertiesPrimarySerializer;
		class BasicPropertyCacheDelta;
		class BasicPropertyCacheView;
		struct PropertyBaseSetDeltaPointers;
		struct PropertyBaseSets;
		class PropertyCache;
		class PropertyCacheDelta;
		class PropertyCacheView;
		class PropertyPatriciaTree;

		template<typename TCache, typename TCacheDelta, typename TKey, typename TGetResult>
		class ReadOnlyArtifactCache;
	}
}

namespace catapult { namespace cache {

	/// Describes a property cache.
	struct PropertyCacheDescriptor {
	public:
		static constexpr auto Name = "PropertyCache";

	public:
		// key value types
		using KeyType = Address;
		using ValueType = state::AccountProperties;

		// cache types
		using CacheType = PropertyCache;
		using CacheDeltaType = PropertyCacheDelta;
		using CacheViewType = PropertyCacheView;

		using Serializer = AccountPropertiesPrimarySerializer;
		using PatriciaTree = PropertyPatriciaTree;

	public:
		/// Gets the key corresponding to \a entry.
		static const auto& GetKeyFromValue(const ValueType& entry) {
			return entry.address();
		}
	};

	/// Property cache types.
	struct PropertyCacheTypes {
		using PrimaryTypes = MutableUnorderedMapAdapter<PropertyCacheDescriptor, utils::ArrayHasher<Address>>;

		using CacheReadOnlyType = ReadOnlyArtifactCache<
			BasicPropertyCacheView,
			BasicPropertyCacheDelta,
			const Address&,
			state::AccountProperties>;

		using BaseSetDeltaPointers = PropertyBaseSetDeltaPointers;
		using BaseSets = PropertyBaseSets;
	};
}}
