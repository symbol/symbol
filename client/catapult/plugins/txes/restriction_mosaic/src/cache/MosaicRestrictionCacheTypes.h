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
#include "src/state/MosaicRestrictionEntry.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/cache/SingleSetCacheTypesAdapter.h"
#include "catapult/utils/Hashers.h"

namespace catapult {
	namespace cache {
		class BasicMosaicRestrictionCacheDelta;
		class BasicMosaicRestrictionCacheView;
		struct MosaicRestrictionBaseSetDeltaPointers;
		struct MosaicRestrictionBaseSets;
		class MosaicRestrictionCache;
		class MosaicRestrictionCacheDelta;
		class MosaicRestrictionCacheView;
		struct MosaicRestrictionEntryPrimarySerializer;
		class MosaicRestrictionPatriciaTree;
		class ReadOnlyMosaicRestrictionCache;
	}
}

namespace catapult { namespace cache {

	/// Describes a mosaic restriction cache.
	struct MosaicRestrictionCacheDescriptor {
	public:
		static constexpr auto Name = "MosaicRestrictionCache";

	public:
		// key value types
		using KeyType = Hash256;
		using ValueType = state::MosaicRestrictionEntry;

		// cache types
		using CacheType = MosaicRestrictionCache;
		using CacheDeltaType = MosaicRestrictionCacheDelta;
		using CacheViewType = MosaicRestrictionCacheView;

		using Serializer = MosaicRestrictionEntryPrimarySerializer;
		using PatriciaTree = MosaicRestrictionPatriciaTree;

	public:
		/// Gets the key corresponding to \a entry.
		static auto GetKeyFromValue(const ValueType& entry) {
			return entry.uniqueKey();
		}
	};

	/// Mosaic restriction cache types.
	struct MosaicRestrictionCacheTypes {
		using PrimaryTypes = MutableUnorderedMapAdapter<MosaicRestrictionCacheDescriptor, utils::ArrayHasher<Hash256>>;

		using CacheReadOnlyType = ReadOnlyMosaicRestrictionCache;

		using BaseSetDeltaPointers = MosaicRestrictionBaseSetDeltaPointers;
		using BaseSets = MosaicRestrictionBaseSets;
	};
}}
