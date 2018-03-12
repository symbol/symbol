#pragma once
#include "src/cache/HashLockInfoCacheTypes.h"
#include "src/cache/SecretLockInfoCacheTypes.h"
#include "src/storages/MongoLockInfoCacheStorage.h"
#include "tests/test/LockInfoCacheTestUtils.h"

namespace catapult { namespace mongo { class MongoStorageContext; } }

namespace catapult { namespace test {

	/// Mongo traits for a hash lock info.
	struct MongoHashLockInfoTestTraits : public BasicHashLockInfoTestTraits {
		/// The number of additional fields
		static const size_t Num_Additional_Fields = 1;

		/// Sets the \a key of the \a lockInfo.
		static void SetKey(ValueType& lockInfo, const KeyType& key);

		/// Creates a catapult cache.
		static cache::CatapultCache CreateCatapultCache();

		/// Creates a mongo hash lock info cache storage around \a context.
		static std::unique_ptr<mongo::ExternalCacheStorage> CreateMongoCacheStorage(mongo::MongoStorageContext& context);
	};

	/// Mongo traits for a secret lock info.
	struct MongoSecretLockInfoTestTraits : public BasicSecretLockInfoTestTraits {
		/// The number of additional fields
		static const size_t Num_Additional_Fields = 3;

		/// Sets the \a key of the \a lockInfo.
		static void SetKey(ValueType& lockInfo, const KeyType& key);

		/// Creates a catapult cache.
		static cache::CatapultCache CreateCatapultCache();

		/// Creates a mongo secret lock info cache storage around \a context.
		static std::unique_ptr<mongo::ExternalCacheStorage> CreateMongoCacheStorage(mongo::MongoStorageContext& context);
	};
}}
