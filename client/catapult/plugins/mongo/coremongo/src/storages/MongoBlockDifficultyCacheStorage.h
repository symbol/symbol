#pragma once
#include "src/ExternalCacheStorage.h"
#include <memory>

namespace catapult { namespace mongo { namespace plugins { class MongoDatabase; } } }

namespace catapult { namespace mongo { namespace storages {

	/// Creates a mongo block difficulty cache storage around \a database and \a difficultyHistorySize.
	std::unique_ptr<plugins::ExternalCacheStorage> CreateMongoBlockDifficultyCacheStorage(
			plugins::MongoDatabase&& database,
			uint64_t difficultyHistorySize);
}}}
