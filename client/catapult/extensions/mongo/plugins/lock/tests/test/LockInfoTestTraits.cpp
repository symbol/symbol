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
