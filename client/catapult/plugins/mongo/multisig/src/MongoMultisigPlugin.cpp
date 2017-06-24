#include "ModifyMultisigAccountMapper.h"
#include "storages/MongoMultisigCacheStorage.h"
#include "plugins/mongo/coremongo/src/MongoPluginManager.h"

extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::plugins::MongoPluginManager& manager) {
	// transaction support
	manager.addTransactionSupport(catapult::mongo::plugins::CreateModifyMultisigAccountTransactionMongoPlugin());

	// cache storage support
	manager.addStorageSupport(catapult::mongo::storages::CreateMongoMultisigCacheStorage(
			manager.createDatabaseConnection(),
			manager.mongoConfig().bulkWriter()));
}
