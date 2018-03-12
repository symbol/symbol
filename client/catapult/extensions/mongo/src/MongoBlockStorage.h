#pragma once
#include "MongoStorageContext.h"
#include "catapult/io/BlockStorage.h"

namespace catapult { namespace mongo { class MongoTransactionRegistry; } }

namespace catapult { namespace mongo {

	/// Creates a mongodb block storage around \a context and \a transactionRegistry.
	std::unique_ptr<io::LightBlockStorage> CreateMongoBlockStorage(
			MongoStorageContext& context,
			const MongoTransactionRegistry& transactionRegistry);
}}
