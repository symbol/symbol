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

#include "MongoLockInfoCacheStorage.h"
#include "src/mappers/LockInfoMapper.h"
#include "mongo/src/storages/MongoCacheStorage.h"
#include "plugins/txes/lock/src/cache/HashLockInfoCache.h"
#include "plugins/txes/lock/src/cache/SecretLockInfoCache.h"
#include "catapult/model/Address.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct HashLockCacheTraits : public storages::BasicMongoCacheStorageTraits<cache::HashLockInfoCacheDescriptor> {
			static constexpr const char* Collection_Name = "hashLockInfos";
			static constexpr const char* Id_Property_Name = "lock.hash";
		};

		struct SecretLockCacheTraits : public storages::BasicMongoCacheStorageTraits<cache::SecretLockInfoCacheDescriptor> {
			static constexpr const char* Collection_Name = "secretLockInfos";
			static constexpr const char* Id_Property_Name = "lock.secret";
		};

		template<typename TLockInfoTraits>
		struct LockInfoCacheTraits : public TLockInfoTraits {
			using CacheDeltaType = typename TLockInfoTraits::CacheDeltaType;
			using KeyType = typename TLockInfoTraits::KeyType;
			using ModelType = typename TLockInfoTraits::ModelType;

			static auto MapToMongoId(const KeyType& key) {
				return mappers::ToBinary(key);
			}

			static auto MapToMongoDocument(const ModelType& lockInfo, model::NetworkIdentifier networkIdentifier) {
				return plugins::ToDbModel(lockInfo, model::PublicKeyToAddress(lockInfo.Account, networkIdentifier));
			}

			static void Insert(CacheDeltaType& cache, const bsoncxx::document::view& document) {
				ModelType lockInfo;
				ToLockInfo(document, lockInfo);
				cache.insert(lockInfo);
			}
		};
	}

	DEFINE_MONGO_FLAT_CACHE_STORAGE(HashLockInfo, LockInfoCacheTraits<HashLockCacheTraits>)

	DEFINE_MONGO_FLAT_CACHE_STORAGE(SecretLockInfo, LockInfoCacheTraits<SecretLockCacheTraits>)
}}}
