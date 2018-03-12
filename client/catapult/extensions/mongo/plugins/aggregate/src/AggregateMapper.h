#pragma once
#include "mongo/src/MongoTransactionPlugin.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Creates a mongo aggregate transaction plugin around \a transactionRegistry for transactions with type \a transactionType.
	PLUGIN_API
	std::unique_ptr<MongoTransactionPlugin> CreateAggregateTransactionMongoPlugin(
			const MongoTransactionRegistry& transactionRegistry,
			model::EntityType transactionType);
}}}
