#pragma once
#include "mongo/src/CacheStorageInclude.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Creates a mongo mosaic cache storage around \a database, \a bulkWriter and \a networkIdentifier.
	DECLARE_MONGO_CACHE_STORAGE(Mosaic);
}}}
