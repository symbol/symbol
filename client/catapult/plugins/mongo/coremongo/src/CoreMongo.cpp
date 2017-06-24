#include "CoreMongo.h"
#include "MongoPluginManager.h"
#include "storages/MongoAccountStateCacheStorage.h"
#include "storages/MongoBlockDifficultyCacheStorage.h"
#include "catapult/model/BlockChainConfiguration.h"

namespace catapult { namespace mongo { namespace plugins {

	void RegisterCoreMongoSystem(MongoPluginManager& manager) {
		manager.addStorageSupport(storages::CreateMongoAccountStateCacheStorage(
				manager.createDatabaseConnection(),
				manager.mongoConfig().bulkWriter()));
		manager.addStorageSupport(storages::CreateMongoBlockDifficultyCacheStorage(
				manager.createDatabaseConnection(),
				model::CalculateDifficultyHistorySize(manager.chainConfig())));
	}
}}}
