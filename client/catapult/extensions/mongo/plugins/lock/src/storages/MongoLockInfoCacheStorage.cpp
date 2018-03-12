#include "MongoLockInfoCacheStorage.h"
#include "src/mappers/LockInfoMapper.h"
#include "mongo/src/storages/MongoCacheStorage.h"
#include "plugins/txes/lock/src/cache/HashLockInfoCache.h"
#include "plugins/txes/lock/src/cache/SecretLockInfoCache.h"
#include "catapult/model/Address.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct HashLockCacheTraits : public storages::BasicMongoCacheStorageTraits<cache::HashLockInfoCacheDescriptor> {
			static constexpr const char* Collection_Name = "hashLockInfos";
			static constexpr const char* Id_Property_Name = "lock.hash";
		};

		struct SecretLockCacheTraits : public storages::BasicMongoCacheStorageTraits<cache::SecretLockInfoCacheDescriptor> {
			static constexpr const char* Collection_Name = "secretLockInfos";
			static constexpr const char* Id_Property_Name = "lock.secret";
		};

		template<typename TLockInfoTraits>
		struct LockInfoCacheTraits : public TLockInfoTraits {
			using CacheDeltaType = typename TLockInfoTraits::CacheDeltaType;
			using KeyType = typename TLockInfoTraits::KeyType;
			using ModelType = typename TLockInfoTraits::ModelType;

			static auto MapToMongoId(const KeyType& key) {
				return mappers::ToBinary(key);
			}

			static auto MapToMongoDocument(const ModelType& lockInfo, model::NetworkIdentifier networkIdentifier) {
				return plugins::ToDbModel(lockInfo, model::PublicKeyToAddress(lockInfo.Account, networkIdentifier));
			}

			static void Insert(CacheDeltaType& cache, const bsoncxx::document::view& document) {
				ModelType lockInfo;
				ToLockInfo(document, lockInfo);
				cache.insert(lockInfo);
			}
		};
	}

	DEFINE_MONGO_FLAT_CACHE_STORAGE(HashLockInfo, LockInfoCacheTraits<HashLockCacheTraits>)

	DEFINE_MONGO_FLAT_CACHE_STORAGE(SecretLockInfo, LockInfoCacheTraits<SecretLockCacheTraits>)
}}}
