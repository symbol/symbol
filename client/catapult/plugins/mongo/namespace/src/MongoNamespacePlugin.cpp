#include "MosaicDefinitionMapper.h"
#include "MosaicSupplyChangeMapper.h"
#include "RegisterNamespaceMapper.h"
#include "storages/MongoMosaicCacheStorage.h"
#include "storages/MongoNamespaceCacheStorage.h"
#include "plugins/mongo/coremongo/src/MongoPluginManager.h"

extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::plugins::MongoPluginManager& manager) {
	// transaction support
	manager.addTransactionSupport(catapult::mongo::plugins::CreateMosaicDefinitionTransactionMongoPlugin());
	manager.addTransactionSupport(catapult::mongo::plugins::CreateMosaicSupplyChangeTransactionMongoPlugin());
	manager.addTransactionSupport(catapult::mongo::plugins::CreateRegisterNamespaceTransactionMongoPlugin());

	// cache storage support
	manager.addStorageSupport(catapult::mongo::storages::CreateMongoNamespaceCacheStorage(
			manager.createDatabaseConnection(),
			manager.mongoConfig().bulkWriter()));
	manager.addStorageSupport(catapult::mongo::storages::CreateMongoMosaicCacheStorage(
			manager.createDatabaseConnection(),
			manager.mongoConfig().bulkWriter()));
}
