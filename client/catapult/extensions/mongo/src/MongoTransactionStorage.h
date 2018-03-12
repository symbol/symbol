#pragma once
#include "MongoStorageContext.h"
#include "catapult/cache/UtChangeSubscriber.h"

namespace catapult { namespace mongo { class MongoTransactionRegistry; } }

namespace catapult { namespace mongo {

	/// Creates a mongodb transaction storage around \a context and \a transactionRegistry for collection with name \a collectionName.
	std::unique_ptr<cache::UtChangeSubscriber> CreateMongoTransactionStorage(
			MongoStorageContext& context,
			const MongoTransactionRegistry& transactionRegistry,
			const std::string& collectionName);
}}
