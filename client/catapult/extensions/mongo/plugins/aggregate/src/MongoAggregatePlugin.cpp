#include "AggregateMapper.h"
#include "mongo/src/MongoPluginManager.h"
#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"

extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::MongoPluginManager& manager) {
	using namespace catapult::model;
	using catapult::mongo::plugins::CreateAggregateTransactionMongoPlugin;

	manager.addTransactionSupport(CreateAggregateTransactionMongoPlugin(manager.transactionRegistry(), Entity_Type_Aggregate_Complete));
	manager.addTransactionSupport(CreateAggregateTransactionMongoPlugin(manager.transactionRegistry(), Entity_Type_Aggregate_Bonded));
}
