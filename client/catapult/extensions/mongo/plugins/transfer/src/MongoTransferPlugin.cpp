#include "TransferMapper.h"
#include "mongo/src/MongoPluginManager.h"

extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::MongoPluginManager& manager) {
	manager.addTransactionSupport(catapult::mongo::plugins::CreateTransferTransactionMongoPlugin());
}
