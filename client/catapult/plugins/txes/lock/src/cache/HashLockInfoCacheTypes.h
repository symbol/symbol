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
#include "LockInfoCacheDelta.h"
#include "LockInfoCacheTypes.h"
#include "LockInfoCacheView.h"
#include "src/model/LockInfo.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"

namespace catapult {
	namespace cache {
		class HashLockInfoCache;
		struct HashLockInfoCacheTypes;
	}
}

namespace catapult { namespace cache {

	/// Describes a hash lock info cache.
	struct HashLockInfoCacheDescriptor {
	public:
		static constexpr auto Name = "HashLockInfoCache";

	public:
		// key value types
		using KeyType = Hash256;
		using ValueType = model::HashLockInfo;

		// cache types
		using CacheType = HashLockInfoCache;
		using CacheDeltaType = LockInfoCacheDelta<HashLockInfoCacheDescriptor, HashLockInfoCacheTypes>;
		using CacheViewType = LockInfoCacheView<HashLockInfoCacheDescriptor, HashLockInfoCacheTypes>;

	public:
		/// Gets the key corresponding to \a lockInfo.
		static const auto& GetKeyFromValue(const ValueType& lockInfo) {
			return lockInfo.Hash;
		}
	};

	/// Hash lock info cache types.
	struct HashLockInfoCacheTypes : public LockInfoCacheTypes<HashLockInfoCacheDescriptor> {
		using BasicCacheDeltaType = BasicLockInfoCacheDelta<HashLockInfoCacheDescriptor, HashLockInfoCacheTypes>;
		using BasicCacheViewType = BasicLockInfoCacheView<HashLockInfoCacheDescriptor, HashLockInfoCacheTypes>;

		using CacheReadOnlyType = ReadOnlyArtifactCache<BasicCacheViewType, BasicCacheDeltaType, Hash256, const model::HashLockInfo&>;
	};
}}
