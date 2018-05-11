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
#include "src/state/Namespace.h"
#include "src/state/NamespaceEntry.h"
#include "src/state/RootNamespaceHistory.h"
#include "catapult/cache/CacheDatabaseMixin.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/IdentifierGroup.h"

namespace catapult {
	namespace cache {
		class BasicNamespaceCacheDelta;
		class BasicNamespaceCacheView;
		class NamespaceCache;
		class NamespaceCacheDelta;
		class NamespaceCacheView;

		template<typename TCache, typename TCacheDelta, typename TKey, typename TGetResult>
		class ReadOnlyArtifactCache;
	}
}

namespace catapult { namespace cache {

	/// Describes a namespace cache.
	struct NamespaceCacheDescriptor {
	public:
		// key value types
		using KeyType = NamespaceId;
		using ValueType = state::RootNamespaceHistory;

		// cache types
		using CacheType = NamespaceCache;
		using CacheDeltaType = NamespaceCacheDelta;
		using CacheViewType = NamespaceCacheView;

	public:
		/// Gets the key corresponding to \a history.
		static auto GetKeyFromValue(const ValueType& history) {
			return history.id();
		}
	};

	/// Namespace cache types.
	struct NamespaceCacheTypes {
	public:
		using CacheReadOnlyType = ReadOnlyArtifactCache<
			BasicNamespaceCacheView,
			BasicNamespaceCacheDelta,
			NamespaceId,
			state::NamespaceEntry>;

	// region secondary descriptors

	private:
		struct FlatMapTypesDescriptor {
		public:
			using KeyType = NamespaceId;
			using ValueType = state::Namespace;

		public:
			static auto GetKeyFromValue(const ValueType& ns) {
				return ns.id();
			}
		};

		struct HeightGroupingTypesDescriptor {
		public:
			using KeyType = Height;
			using ValueType = utils::IdentifierGroup<NamespaceId, Height, utils::BaseValueHasher<NamespaceId>>;

		public:
			static auto GetKeyFromValue(const ValueType& heightNamespaces) {
				return heightNamespaces.key();
			}
		};

	// endregion

	public:
		using PrimaryTypes = MutableUnorderedMapAdapter<NamespaceCacheDescriptor, utils::BaseValueHasher<NamespaceId>>;
		using FlatMapTypes = MutableUnorderedMapAdapter<FlatMapTypesDescriptor, utils::BaseValueHasher<NamespaceId>>;
		using HeightGroupingTypes = MutableUnorderedMapAdapter<HeightGroupingTypesDescriptor, utils::BaseValueHasher<Height>>;

	public:
		// in order to compose namespace cache from multiple sets, define an aggregate set type

		struct BaseSetDeltaPointers {
			PrimaryTypes::BaseSetDeltaPointerType pPrimary;
			FlatMapTypes::BaseSetDeltaPointerType pFlatMap;
			HeightGroupingTypes::BaseSetDeltaPointerType pHeightGrouping;
		};

		struct BaseSets : public CacheDatabaseMixin {
		public:
			explicit BaseSets(const CacheConfiguration& config)
					: CacheDatabaseMixin(config, { "default", "flat_map", "height_grouping" })
					, Primary(GetContainerMode(config), database(), 0)
					, FlatMap(GetContainerMode(config), database(), 1)
					, HeightGrouping(GetContainerMode(config), database(), 2)
			{}

		public:
			PrimaryTypes::BaseSetType Primary;
			FlatMapTypes::BaseSetType FlatMap;
			HeightGroupingTypes::BaseSetType HeightGrouping;

		public:
			BaseSetDeltaPointers rebase() {
				return { Primary.rebase(), FlatMap.rebase(), HeightGrouping.rebase() };
			}

			BaseSetDeltaPointers rebaseDetached() const {
				return { Primary.rebaseDetached(), FlatMap.rebaseDetached(), HeightGrouping.rebaseDetached() };
			}

			void commit() {
				Primary.commit();
				FlatMap.commit();
				HeightGrouping.commit();
			}
		};
	};
}}
