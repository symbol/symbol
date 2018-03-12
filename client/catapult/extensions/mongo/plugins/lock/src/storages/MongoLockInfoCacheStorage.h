#pragma once
#include "mongo/src/CacheStorageInclude.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Creates a mongo hash lock info cache storage around \a database, \a bulkWriter and \a networkIdentifier.
	DECLARE_MONGO_CACHE_STORAGE(HashLockInfo);

	/// Creates a mongo secret lock info cache storage around \a database, \a bulkWriter and \a networkIdentifier.
	DECLARE_MONGO_CACHE_STORAGE(SecretLockInfo);
}}}
