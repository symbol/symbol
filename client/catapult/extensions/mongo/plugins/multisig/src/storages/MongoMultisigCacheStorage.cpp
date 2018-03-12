#include "MongoMultisigCacheStorage.h"
#include "src/mappers/MultisigEntryMapper.h"
#include "mongo/src/storages/MongoCacheStorage.h"
#include "plugins/txes/multisig/src/cache/MultisigCache.h"
#include "catapult/model/Address.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct MultisigCacheTraits : public storages::BasicMongoCacheStorageTraits<cache::MultisigCacheDescriptor> {
			static constexpr const char* Collection_Name = "multisigs";
			static constexpr const char* Id_Property_Name = "multisig.account";

			static auto MapToMongoId(const KeyType& key) {
				return mappers::ToBinary(key);
			}

			static auto MapToMongoDocument(const ModelType& entry, model::NetworkIdentifier networkIdentifier) {
				return plugins::ToDbModel(entry, model::PublicKeyToAddress(entry.key(), networkIdentifier));
			}

			static void Insert(CacheDeltaType& cache, const bsoncxx::document::view& document) {
				cache.insert(ToMultisigEntry(document));
			}
		};
	}

	DEFINE_MONGO_FLAT_CACHE_STORAGE(Multisig, MultisigCacheTraits)
}}}
