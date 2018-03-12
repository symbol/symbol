#include "HashLockMapper.h"
#include "SecretLockMapper.h"
#include "SecretProofMapper.h"
#include "storages/MongoLockInfoCacheStorage.h"
#include "mongo/src/MongoPluginManager.h"

extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::MongoPluginManager& manager) {
	// transaction support
	manager.addTransactionSupport(catapult::mongo::plugins::CreateHashLockTransactionMongoPlugin());
	manager.addTransactionSupport(catapult::mongo::plugins::CreateSecretLockTransactionMongoPlugin());
	manager.addTransactionSupport(catapult::mongo::plugins::CreateSecretProofTransactionMongoPlugin());

	// cache storage support
	manager.addStorageSupport(catapult::mongo::plugins::CreateMongoHashLockInfoCacheStorage(
			manager.createDatabaseConnection(),
			manager.mongoContext().bulkWriter(),
			manager.chainConfig().Network.Identifier));
	manager.addStorageSupport(catapult::mongo::plugins::CreateMongoSecretLockInfoCacheStorage(
			manager.createDatabaseConnection(),
			manager.mongoContext().bulkWriter(),
			manager.chainConfig().Network.Identifier));
}
