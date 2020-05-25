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
#include "src/state/MosaicEntry.h"
#include "catapult/cache/CacheDatabaseMixin.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/IdentifierGroup.h"

namespace catapult {
	namespace cache {
		class BasicMosaicCacheDelta;
		class BasicMosaicCacheView;
		struct MosaicBaseSetDeltaPointers;
		struct MosaicBaseSets;
		class MosaicCache;
		class MosaicCacheDelta;
		class MosaicCacheView;
		struct MosaicEntryPrimarySerializer;
		struct MosaicHeightGroupingSerializer;
		class MosaicPatriciaTree;

		template<typename TCache, typename TCacheDelta, typename TCacheKey, typename TGetResult>
		class ReadOnlyArtifactCache;
	}
}

namespace catapult { namespace cache {

	/// Describes a mosaic cache.
	struct MosaicCacheDescriptor {
	public:
		static constexpr auto Name = "MosaicCache";

	public:
		// key value types
		using KeyType = MosaicId;
		using ValueType = state::MosaicEntry;

		// cache types
		using CacheType = MosaicCache;
		using CacheDeltaType = MosaicCacheDelta;
		using CacheViewType = MosaicCacheView;

		using Serializer = MosaicEntryPrimarySerializer;
		using PatriciaTree = MosaicPatriciaTree;

	public:
		/// Gets the key corresponding to \a entry.
		static auto GetKeyFromValue(const ValueType& entry) {
			return entry.mosaicId();
		}
	};

	/// Mosaic cache types.
	struct MosaicCacheTypes {
	public:
		using CacheReadOnlyType = ReadOnlyArtifactCache<BasicMosaicCacheView, BasicMosaicCacheDelta, MosaicId, state::MosaicEntry>;

	// region secondary descriptors

	public:
		struct HeightGroupingTypesDescriptor {
		public:
			using KeyType = Height;
			using ValueType = utils::IdentifierGroup<MosaicId, Height, utils::BaseValueHasher<MosaicId>>;
			using Serializer = MosaicHeightGroupingSerializer;

		public:
			static auto GetKeyFromValue(const ValueType& heightMosaics) {
				return heightMosaics.key();
			}
		};

	// endregion

	public:
		using PrimaryTypes = MutableUnorderedMapAdapter<MosaicCacheDescriptor, utils::BaseValueHasher<MosaicId>>;
		using HeightGroupingTypes = MutableUnorderedMapAdapter<HeightGroupingTypesDescriptor, utils::BaseValueHasher<Height>>;

	public:
		using BaseSetDeltaPointers = MosaicBaseSetDeltaPointers;
		using BaseSets = MosaicBaseSets;
	};
}}
