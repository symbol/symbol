#pragma once
#include "MongoStorageContext.h"
#include "catapult/subscribers/TransactionStatusSubscriber.h"
#include <memory>

namespace catapult { namespace mongo {

	/// Creates a mongodb transaction status storage around \a context.
	std::unique_ptr<subscribers::TransactionStatusSubscriber> CreateMongoTransactionStatusStorage(MongoStorageContext& context);
}}
