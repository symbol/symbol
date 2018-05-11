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

#include "src/storages/MongoLockInfoCacheStorage.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/model/Address.h"
#include "mongo/tests/test/MongoFlatCacheStorageTests.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "plugins/txes/lock/tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/LockMapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoLockInfoCacheStorageTests

	namespace {
		struct HashLockCacheTraits {
			using CacheType = cache::HashLockInfoCache;
			using ModelType = model::HashLockInfo;

			static constexpr auto Collection_Name = "hashLockInfos";
			static constexpr auto Id_Property_Name = "lock.hash";

			static constexpr auto CreateCacheStorage = CreateMongoHashLockInfoCacheStorage;

			static auto GetId(const ModelType& lockInfo) {
				return lockInfo.Hash;
			}

			static cache::CatapultCache CreateCache() {
				return test::HashLockInfoCacheFactory::Create();
			}

			static ModelType GenerateRandomElement(uint32_t id) {
				return test::BasicHashLockInfoTestTraits::CreateLockInfo(Height(id));
			}
		};

		struct SecretLockCacheTraits {
			using CacheType = cache::SecretLockInfoCache;
			using ModelType = model::SecretLockInfo;

			static constexpr auto Collection_Name = "secretLockInfos";
			static constexpr auto Id_Property_Name = "lock.secret";

			static constexpr auto CreateCacheStorage = CreateMongoSecretLockInfoCacheStorage;

			static auto GetId(const ModelType& lockInfo) {
				return lockInfo.Secret;
			}

			static cache::CatapultCache CreateCache() {
				return test::SecretLockInfoCacheFactory::Create();
			}

			static ModelType GenerateRandomElement(uint32_t id) {
				return test::BasicSecretLockInfoTestTraits::CreateLockInfo(Height(id));
			}
		};

		template<typename TLockInfoTraits>
		struct LockInfoCacheTraits : public TLockInfoTraits {
			static constexpr auto Network_Id = static_cast<model::NetworkIdentifier>(0x5A);

			static void Add(cache::CatapultCacheDelta& delta, const typename TLockInfoTraits::ModelType& lockInfo) {
				auto& lockInfoCacheDelta = delta.sub<typename TLockInfoTraits::CacheType>();
				lockInfoCacheDelta.insert(lockInfo);
			}

			static void Remove(cache::CatapultCacheDelta& delta, const typename TLockInfoTraits::ModelType& lockInfo) {
				auto& lockInfoCacheDelta = delta.sub<typename TLockInfoTraits::CacheType>();
				lockInfoCacheDelta.remove(TLockInfoTraits::GetId(lockInfo));
			}

			static void Mutate(cache::CatapultCacheDelta& delta, typename TLockInfoTraits::ModelType& lockInfo) {
				// update expected
				lockInfo.Status = model::LockStatus::Used;

				// update cache
				auto& lockInfoCacheDelta = delta.sub<typename TLockInfoTraits::CacheType>();
				lockInfoCacheDelta.get(TLockInfoTraits::GetId(lockInfo)).Status = model::LockStatus::Used;
			}

			static auto GetFindFilter(const typename TLockInfoTraits::ModelType& lockInfo) {
				return document()
						<< std::string(TLockInfoTraits::Id_Property_Name)
						<< mappers::ToBinary(TLockInfoTraits::GetId(lockInfo))
						<< finalize;
			}

			static void AssertEqual(const typename TLockInfoTraits::ModelType& lockInfo, const bsoncxx::document::view& view) {
				auto address = model::PublicKeyToAddress(lockInfo.Account, Network_Id);
				test::AssertEqualLockInfoData(lockInfo, address, view["lock"].get_document().view());
			}
		};
	}

	DEFINE_FLAT_CACHE_STORAGE_TESTS(LockInfoCacheTraits<HashLockCacheTraits>, _Hash)
	DEFINE_FLAT_CACHE_STORAGE_TESTS(LockInfoCacheTraits<SecretLockCacheTraits>, _Secret)
}}}
