#pragma once
#include "ChainScoreProvider.h"
#include "MongoStorageContext.h"

namespace catapult { namespace mongo {

	/// Creates a mongodb chain score provider around \a context.
	std::unique_ptr<ChainScoreProvider> CreateMongoChainScoreProvider(MongoStorageContext& context);
}}
