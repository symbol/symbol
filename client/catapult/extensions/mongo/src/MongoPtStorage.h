#pragma once
#include "MongoStorageContext.h"
#include "catapult/cache/PtChangeSubscriber.h"

namespace catapult { namespace mongo { class MongoTransactionRegistry; } }

namespace catapult { namespace mongo {

	/// Creates a mongodb partial transaction storage around \a context and \a transactionRegistry.
	std::unique_ptr<cache::PtChangeSubscriber> CreateMongoPtStorage(
			MongoStorageContext& context,
			const MongoTransactionRegistry& transactionRegistry);
}}
