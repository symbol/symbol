#include "ModifyMultisigAccountMapper.h"
#include "storages/MongoMultisigCacheStorage.h"
#include "mongo/src/MongoPluginManager.h"

extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::MongoPluginManager& manager) {
	// transaction support
	manager.addTransactionSupport(catapult::mongo::plugins::CreateModifyMultisigAccountTransactionMongoPlugin());

	// cache storage support
	manager.addStorageSupport(catapult::mongo::plugins::CreateMongoMultisigCacheStorage(
			manager.createDatabaseConnection(),
			manager.mongoContext().bulkWriter(),
			manager.chainConfig().Network.Identifier));
}
