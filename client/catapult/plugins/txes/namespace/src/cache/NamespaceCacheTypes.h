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
#include "../state/Namespace.h"
#include "../state/NamespaceEntry.h"
#include "../state/RootNamespaceHistory.h"
#include "src/catapult/cache/CacheDatabaseMixin.h"
#include "src/catapult/cache/CacheDescriptorAdapters.h"
#include "src/catapult/deltaset/BaseSetDelta.h"
#include "src/catapult/utils/Hashers.h"
#include "src/catapult/utils/IdentifierGroup.h"

namespace catapult {
	namespace cache {
		class BasicNamespaceCacheDelta;
		class BasicNamespaceCacheView;
		struct NamespaceBaseSetDeltaPointers;
		struct NamespaceBaseSets;
		class NamespaceCache;
		class NamespaceCacheDelta;
		class NamespaceCacheView;
		struct NamespaceFlatMapTypesSerializer;
		struct NamespaceHeightGroupingSerializer;
		class NamespacePatriciaTree;
		class ReadOnlyNamespaceCache;
		struct RootNamespaceHistoryPrimarySerializer;
	}
}

namespace catapult { namespace cache {

	/// Describes a namespace cache.
	struct NamespaceCacheDescriptor {
	public:
		static constexpr auto Name = "NamespaceCache";

	public:
		// key value types
		using KeyType = NamespaceId;
		using ValueType = state::RootNamespaceHistory;

		// cache types
		using CacheType = NamespaceCache;
		using CacheDeltaType = NamespaceCacheDelta;
		using CacheViewType = NamespaceCacheView;

		using Serializer = RootNamespaceHistoryPrimarySerializer;
		using PatriciaTree = NamespacePatriciaTree;

	public:
		/// Gets the key corresponding to \a history.
		static auto GetKeyFromValue(const ValueType& history) {
			return history.id();
		}
	};

	/// Namespace cache types.
	struct NamespaceCacheTypes {
	public:
		using CacheReadOnlyType = ReadOnlyNamespaceCache;

		/// Custom sub view options.
		struct Options {
			/// Namespace grace period duration.
			BlockDuration GracePeriodDuration;
		};

	// region secondary descriptors

	private:
		struct FlatMapTypesDescriptor {
		public:
			using KeyType = NamespaceId;
			using ValueType = state::Namespace;
			using Serializer = NamespaceFlatMapTypesSerializer;

		public:
			static auto GetKeyFromValue(const ValueType& ns) {
				return ns.id();
			}
		};

	public:
		struct HeightGroupingTypesDescriptor {
		public:
			using KeyType = Height;
			using ValueType = utils::IdentifierGroup<NamespaceId, Height, utils::BaseValueHasher<NamespaceId>>;
			using Serializer = NamespaceHeightGroupingSerializer;

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
		using BaseSetDeltaPointers = NamespaceBaseSetDeltaPointers;
		using BaseSets = NamespaceBaseSets;
	};
}}
