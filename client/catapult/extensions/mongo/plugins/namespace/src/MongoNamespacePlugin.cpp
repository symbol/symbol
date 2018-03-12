#include "MosaicDefinitionMapper.h"
#include "MosaicSupplyChangeMapper.h"
#include "RegisterNamespaceMapper.h"
#include "storages/MongoMosaicCacheStorage.h"
#include "storages/MongoNamespaceCacheStorage.h"
#include "mongo/src/MongoPluginManager.h"

extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::MongoPluginManager& manager) {
	// transaction support
	manager.addTransactionSupport(catapult::mongo::plugins::CreateMosaicDefinitionTransactionMongoPlugin());
	manager.addTransactionSupport(catapult::mongo::plugins::CreateMosaicSupplyChangeTransactionMongoPlugin());
	manager.addTransactionSupport(catapult::mongo::plugins::CreateRegisterNamespaceTransactionMongoPlugin());

	// cache storage support
	manager.addStorageSupport(catapult::mongo::plugins::CreateMongoNamespaceCacheStorage(
			manager.createDatabaseConnection(),
			manager.mongoContext().bulkWriter(),
			manager.chainConfig().Network.Identifier));
	manager.addStorageSupport(catapult::mongo::plugins::CreateMongoMosaicCacheStorage(
			manager.createDatabaseConnection(),
			manager.mongoContext().bulkWriter(),
			manager.chainConfig().Network.Identifier));
}
