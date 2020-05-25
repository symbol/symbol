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
#include "src/state/MultisigEntry.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/utils/Hashers.h"

namespace catapult {
	namespace cache {
		class BasicMultisigCacheDelta;
		class BasicMultisigCacheView;
		struct MultisigBaseSetDeltaPointers;
		struct MultisigBaseSets;
		class MultisigCache;
		class MultisigCacheDelta;
		class MultisigCacheView;
		struct MultisigEntryPrimarySerializer;
		class MultisigPatriciaTree;

		template<typename TCache, typename TCacheDelta, typename TCacheKey, typename TGetResult>
		class ReadOnlyArtifactCache;
	}
}

namespace catapult { namespace cache {

	/// Describes a multisig cache.
	struct MultisigCacheDescriptor {
	public:
		static constexpr auto Name = "MultisigCache";

	public:
		// key value types
		using KeyType = Address;
		using ValueType = state::MultisigEntry;

		// cache types
		using CacheType = MultisigCache;
		using CacheDeltaType = MultisigCacheDelta;
		using CacheViewType = MultisigCacheView;

		using Serializer = MultisigEntryPrimarySerializer;
		using PatriciaTree = MultisigPatriciaTree;

	public:
		/// Gets the key corresponding to \a entry.
		static const auto& GetKeyFromValue(const ValueType& entry) {
			return entry.address();
		}
	};

	/// Multisig cache types.
	struct MultisigCacheTypes {
		using PrimaryTypes = MutableUnorderedMapAdapter<MultisigCacheDescriptor, utils::ArrayHasher<Address>>;

		using CacheReadOnlyType = ReadOnlyArtifactCache<BasicMultisigCacheView, BasicMultisigCacheDelta, Address, state::MultisigEntry>;

		using BaseSetDeltaPointers = MultisigBaseSetDeltaPointers;
		using BaseSets = MultisigBaseSets;
	};
}}
