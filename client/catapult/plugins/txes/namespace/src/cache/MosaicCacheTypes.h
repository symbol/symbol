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
#include "src/state/MosaicHistory.h"
#include "catapult/cache/CacheDatabaseMixin.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/IdentifierGroup.h"

namespace catapult {
	namespace cache {
		class BasicMosaicCacheDelta;
		class BasicMosaicCacheView;
		class MosaicCache;
		class MosaicCacheDelta;
		class MosaicCacheView;

		template<typename TCache, typename TCacheDelta, typename TKey, typename TGetResult>
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
		using ValueType = state::MosaicHistory;

		// cache types
		using CacheType = MosaicCache;
		using CacheDeltaType = MosaicCacheDelta;
		using CacheViewType = MosaicCacheView;

	public:
		/// Gets the key corresponding to \a history.
		static auto GetKeyFromValue(const ValueType& history) {
			return history.id();
		}
	};

	/// Mosaic cache types.
	struct MosaicCacheTypes {
	public:
		using CacheReadOnlyType = ReadOnlyArtifactCache<
			BasicMosaicCacheView,
			BasicMosaicCacheDelta,
			MosaicId,
			const state::MosaicEntry&>;

	// region value adapters

	private:
		template<typename TSource, typename TDest>
		struct ValueAdapter {
			using AdaptedValueType = TDest;

			static TDest& Adapt(TSource& history) {
				return history.back();
			}
		};

	public:
		using ConstValueAdapter = ValueAdapter<const state::MosaicHistory, const state::MosaicEntry>;
		using MutableValueAdapter = ValueAdapter<state::MosaicHistory, state::MosaicEntry>;

	// endregion

	// region secondary descriptors

	private:
		struct NamespaceGroupingTypesDescriptor {
		public:
			using KeyType = NamespaceId;
			using ValueType = utils::IdentifierGroup<MosaicId, NamespaceId, utils::BaseValueHasher<MosaicId>>;

		public:
			static auto GetKeyFromValue(const ValueType& namespaceMosaics) {
				return namespaceMosaics.key();
			}
		};

		struct HeightGroupingTypesDescriptor {
		public:
			using KeyType = Height;
			using ValueType = utils::IdentifierGroup<MosaicId, Height, utils::BaseValueHasher<MosaicId>>;

		public:
			static auto GetKeyFromValue(const ValueType& heightMosaics) {
				return heightMosaics.key();
			}
		};

	// endregion

	public:
		using PrimaryTypes = MutableUnorderedMapAdapter<MosaicCacheDescriptor, utils::BaseValueHasher<MosaicId>>;
		using NamespaceGroupingTypes = MutableUnorderedMapAdapter<NamespaceGroupingTypesDescriptor, utils::BaseValueHasher<NamespaceId>>;
		using HeightGroupingTypes = MutableUnorderedMapAdapter<HeightGroupingTypesDescriptor, utils::BaseValueHasher<Height>>;

	public:
		// in order to compose mosaic cache from multiple sets, define an aggregate set type

		struct BaseSetDeltaPointers {
			PrimaryTypes::BaseSetDeltaPointerType pPrimary;
			NamespaceGroupingTypes::BaseSetDeltaPointerType pNamespaceGrouping;
			HeightGroupingTypes::BaseSetDeltaPointerType pHeightGrouping;
		};

		struct BaseSets : public CacheDatabaseMixin {
		public:
			explicit BaseSets(const CacheConfiguration& config)
					: CacheDatabaseMixin(config, { "default", "namespace_grouping", "height_grouping" })
					, Primary(GetContainerMode(config), database(), 0)
					, NamespaceGrouping(GetContainerMode(config), database(), 1)
					, HeightGrouping(GetContainerMode(config), database(), 2)
			{}

		public:
			PrimaryTypes::BaseSetType Primary;
			NamespaceGroupingTypes::BaseSetType NamespaceGrouping;
			HeightGroupingTypes::BaseSetType HeightGrouping;

		public:
			BaseSetDeltaPointers rebase() {
				return { Primary.rebase(), NamespaceGrouping.rebase(), HeightGrouping.rebase() };
			}

			BaseSetDeltaPointers rebaseDetached() const {
				return { Primary.rebaseDetached(), NamespaceGrouping.rebaseDetached(), HeightGrouping.rebaseDetached() };
			}

			void commit() {
				Primary.commit();
				NamespaceGrouping.commit();
				HeightGrouping.commit();
			}
		};
	};
}}
