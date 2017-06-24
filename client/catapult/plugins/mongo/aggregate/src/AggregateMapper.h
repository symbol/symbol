#pragma once
#include "plugins/mongo/coremongo/src/MongoTransactionPlugin.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Creates a mongo aggregate transaction plugin around \a transactionRegistry.
	PLUGIN_API
	std::unique_ptr<MongoTransactionPlugin> CreateAggregateTransactionMongoPlugin(const MongoTransactionRegistry& transactionRegistry);
}}}
