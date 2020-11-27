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

#include "src/storages/MongoSecretLockInfoCacheStorage.h"
#include "mongo/plugins/lock_shared/tests/int/storages/MongoLockInfoCacheStorageTestTraits.h"
#include "mongo/tests/test/MongoFlatCacheStorageTests.h"
#include "plugins/txes/lock_secret/tests/test/SecretLockInfoCacheTestUtils.h"
#include "tests/test/SecretLockMapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoSecretLockInfoCacheStorageTests

	namespace {
		struct SecretLockCacheTraits {
			using CacheType = cache::SecretLockInfoCache;
			using ModelType = state::SecretLockInfo;

			static constexpr auto Collection_Name = "secretLocks";
			static constexpr auto Id_Property_Name = "compositeHash";

			static constexpr auto CreateCacheStorage = CreateMongoSecretLockInfoCacheStorage;
			static constexpr auto AssertEqualLockInfoData = test::AssertEqualLockInfoData;

			static auto GetId(const ModelType& lockInfo) {
				return lockInfo.CompositeHash;
			}

			static cache::CatapultCache CreateCache() {
				return test::SecretLockInfoCacheFactory::Create();
			}

			static ModelType GenerateRandomElement(uint32_t id) {
				return test::BasicSecretLockInfoTestTraits::CreateLockInfo(Height(id));
			}
		};
	}

	DEFINE_FLAT_CACHE_STORAGE_TESTS(MongoLockInfoCacheStorageTestTraits<SecretLockCacheTraits>,)
}}}
