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
#include "src/state/AccountRestrictions.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/cache/SingleSetCacheTypesAdapter.h"
#include "catapult/utils/Hashers.h"

namespace catapult {
	namespace cache {
		struct AccountRestrictionBaseSetDeltaPointers;
		struct AccountRestrictionBaseSets;
		class AccountRestrictionCache;
		class AccountRestrictionCacheDelta;
		class AccountRestrictionCacheView;
		class AccountRestrictionPatriciaTree;
		struct AccountRestrictionsPrimarySerializer;
		class BasicAccountRestrictionCacheDelta;
		class BasicAccountRestrictionCacheView;

		template<typename TCache, typename TCacheDelta, typename TCacheKey, typename TGetResult>
		class ReadOnlyArtifactCache;
	}
}

namespace catapult { namespace cache {

	/// Describes an account restriction cache.
	struct AccountRestrictionCacheDescriptor {
	public:
		static constexpr auto Name = "AccountRestrictionCache";

	public:
		// key value types
		using KeyType = Address;
		using ValueType = state::AccountRestrictions;

		// cache types
		using CacheType = AccountRestrictionCache;
		using CacheDeltaType = AccountRestrictionCacheDelta;
		using CacheViewType = AccountRestrictionCacheView;

		using Serializer = AccountRestrictionsPrimarySerializer;
		using PatriciaTree = AccountRestrictionPatriciaTree;

	public:
		/// Gets the key corresponding to \a entry.
		static const auto& GetKeyFromValue(const ValueType& entry) {
			return entry.address();
		}
	};

	/// Account restriction cache types.
	struct AccountRestrictionCacheTypes {
		using PrimaryTypes = MutableUnorderedMapAdapter<AccountRestrictionCacheDescriptor, utils::ArrayHasher<Address>>;

		using CacheReadOnlyType = ReadOnlyArtifactCache<
			BasicAccountRestrictionCacheView,
			BasicAccountRestrictionCacheDelta,
			Address,
			state::AccountRestrictions>;

		using BaseSetDeltaPointers = AccountRestrictionBaseSetDeltaPointers;
		using BaseSets = AccountRestrictionBaseSets;
	};
}}
