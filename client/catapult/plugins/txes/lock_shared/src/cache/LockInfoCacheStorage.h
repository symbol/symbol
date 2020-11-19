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
#include "catapult/cache/CacheStorageInclude.h"

namespace catapult { namespace cache {

	/// Policy for saving and loading lock info cache data.
	template<typename TDescriptor, typename TLockInfoSerializer>
	struct LockInfoCacheStorage
			: public CacheStorageFromDescriptor<TDescriptor>
			, public TLockInfoSerializer {
	public:
		/// Loads \a history into \a cacheDelta.
		static void LoadInto(const typename TDescriptor::ValueType& history, typename TDescriptor::CacheDeltaType& cacheDelta) {
			for (const auto& lockInfo : history)
				cacheDelta.insert(lockInfo);
		}

		/// Purges \a history from \a cacheDelta.
		static void Purge(const typename TDescriptor::ValueType& history, typename TDescriptor::CacheDeltaType& cacheDelta) {
			while (cacheDelta.contains(history.id()))
				cacheDelta.remove(history.id());
		}
	};
}}

/// Defines lock info cache storage for \a LOCK_INFO.
#define DEFINE_LOCK_INFO_CACHE_STORAGE(LOCK_INFO) \
	/* Policy for saving and loading lock info cache data. */ \
	struct LOCK_INFO##CacheStorage : public LockInfoCacheStorage<LOCK_INFO##CacheDescriptor, state::LOCK_INFO##HistorySerializer> {};
