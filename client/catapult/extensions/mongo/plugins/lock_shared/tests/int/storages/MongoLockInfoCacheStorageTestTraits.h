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
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/lock_shared/src/state/LockInfo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Mongo lock info cache storage test traits.
	template<typename TLockInfoTraits>
	struct MongoLockInfoCacheStorageTestTraits : public TLockInfoTraits {
		static constexpr auto Network_Id = static_cast<model::NetworkIdentifier>(0x5A);

		/// Adds \a lockInfo to \a delta.
		static void Add(cache::CatapultCacheDelta& delta, const typename TLockInfoTraits::ModelType& lockInfo) {
			auto& lockInfoCacheDelta = delta.sub<typename TLockInfoTraits::CacheType>();
			lockInfoCacheDelta.insert(lockInfo);
		}

		/// Removes \a lockInfo from \a delta.
		static void Remove(cache::CatapultCacheDelta& delta, const typename TLockInfoTraits::ModelType& lockInfo) {
			auto& lockInfoCacheDelta = delta.sub<typename TLockInfoTraits::CacheType>();
			lockInfoCacheDelta.remove(TLockInfoTraits::GetId(lockInfo));
		}

		/// Modifies \a lockInfo in \a delta.
		static void Mutate(cache::CatapultCacheDelta& delta, typename TLockInfoTraits::ModelType& lockInfo) {
			// update expected
			lockInfo.Status = state::LockStatus::Used;

			// update cache
			auto& lockInfoCacheDelta = delta.sub<typename TLockInfoTraits::CacheType>();
			lockInfoCacheDelta.find(TLockInfoTraits::GetId(lockInfo)).get().Status = state::LockStatus::Used;
		}

		/// Gets a filter for finding \a lockInfo.
		static auto GetFindFilter(const typename TLockInfoTraits::ModelType& lockInfo) {
			return mappers::bson_stream::document()
					<< std::string(TLockInfoTraits::Id_Property_Name)
					<< mappers::ToBinary(TLockInfoTraits::GetId(lockInfo))
					<< mappers::bson_stream::finalize;
		}

		/// Asserts that \a lockInfo and \a view are equal.
		static void AssertEqual(const typename TLockInfoTraits::ModelType& lockInfo, const bsoncxx::document::view& view) {
			TLockInfoTraits::AssertEqualLockInfoData(lockInfo, view["lock"].get_document().view());
		}
	};
}}}
