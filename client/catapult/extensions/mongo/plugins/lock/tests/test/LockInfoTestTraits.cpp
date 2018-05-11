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

#include "LockInfoTestTraits.h"
#include "mongo/src/MongoStorageContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region hash based traits

	void MongoHashLockInfoTestTraits::SetKey(ValueType& lockInfo, const KeyType& key) {
		lockInfo.Hash = key;
	}

	cache::CatapultCache MongoHashLockInfoTestTraits::CreateCatapultCache() {
		return HashLockInfoCacheFactory::Create();
	}

	std::unique_ptr<mongo::ExternalCacheStorage> MongoHashLockInfoTestTraits::CreateMongoCacheStorage(
			mongo::MongoStorageContext& context) {
		return mongo::plugins::CreateMongoHashLockInfoCacheStorage(
				context.createDatabaseConnection(),
				context.bulkWriter(),
				model::NetworkIdentifier());
	}

	// endregion

	// region secret based traits

	void MongoSecretLockInfoTestTraits::SetKey(ValueType& lockInfo, const KeyType& key) {
		lockInfo.Secret = key;
	}

	cache::CatapultCache MongoSecretLockInfoTestTraits::CreateCatapultCache() {
		return SecretLockInfoCacheFactory::Create();
	}

	std::unique_ptr<mongo::ExternalCacheStorage> MongoSecretLockInfoTestTraits::CreateMongoCacheStorage(
			mongo::MongoStorageContext& context) {
		return mongo::plugins::CreateMongoSecretLockInfoCacheStorage(
				context.createDatabaseConnection(),
				context.bulkWriter(),
				model::NetworkIdentifier());
	}

	// endregion
}}
