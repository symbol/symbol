#pragma once
#include "ExternalCacheStorage.h"
#include "catapult/model/NetworkInfo.h"
#include <memory>

namespace catapult {
	namespace mongo {
		class MongoBulkWriter;
		class MongoDatabase;
	}
}

/// Declares a mongo cache storage with \a NAME.
#define DECLARE_MONGO_CACHE_STORAGE(NAME) \
	std::unique_ptr<mongo::ExternalCacheStorage> CreateMongo##NAME##CacheStorage( \
			mongo::MongoDatabase&& database, \
			mongo::MongoBulkWriter& bulkWriter, \
			model::NetworkIdentifier networkIdentifier) \

/// Defines a mongo cache storage with \a NAME and \a STORAGE_TYPE using \a TRAITS_NAME.
#define DEFINE_MONGO_CACHE_STORAGE(NAME, STORAGE_TYPE, TRAITS_NAME) \
	DECLARE_MONGO_CACHE_STORAGE(NAME) { \
		return std::make_unique<storages::STORAGE_TYPE<TRAITS_NAME>>( \
				std::move(database), \
				bulkWriter, \
				networkIdentifier); \
	}

/// Defines a mongo flat cache storage with \a NAME using \a TRAITS_NAME.
#define DEFINE_MONGO_FLAT_CACHE_STORAGE(NAME, TRAITS_NAME) \
	DEFINE_MONGO_CACHE_STORAGE(NAME, MongoFlatCacheStorage, TRAITS_NAME)

/// Defines a mongo historical cache storage with \a NAME using \a TRAITS_NAME.
#define DEFINE_MONGO_HISTORICAL_CACHE_STORAGE(NAME, TRAITS_NAME) \
	DEFINE_MONGO_CACHE_STORAGE(NAME, MongoHistoricalCacheStorage, TRAITS_NAME)
