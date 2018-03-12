#pragma once
#include "mongo/src/CacheStorageInclude.h"

namespace catapult { namespace mongo { namespace storages {

	/// Creates a mongo account state cache storage around \a database, \a bulkWriter and \a networkIdentifier.
	DECLARE_MONGO_CACHE_STORAGE(AccountState);
}}}
