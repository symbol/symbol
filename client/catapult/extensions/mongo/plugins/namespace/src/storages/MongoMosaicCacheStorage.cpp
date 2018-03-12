#include "MongoMosaicCacheStorage.h"
#include "CacheStorageUtils.h"
#include "src/mappers/MosaicDescriptorMapper.h"
#include "mongo/src/storages/MongoCacheStorage.h"
#include "plugins/txes/namespace/src/cache/MosaicCache.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct MosaicCacheTraits {
			using CacheType = cache::MosaicCache;
			using CacheDeltaType = cache::MosaicCacheDelta;
			using KeyType = MosaicId;
			using ModelType = MosaicDescriptor;
			using IdContainerType = std::unordered_set<KeyType, utils::BaseValueHasher<KeyType>>;
			using ElementContainerType = std::unordered_set<const state::MosaicHistory*>;

			static constexpr const char* Collection_Name = "mosaics";
			static constexpr const char* Id_Property_Name = "mosaic.mosaicId";

			static auto GetId(const state::MosaicHistory& history) {
				return history.id();
			}

			static auto MapToMongoId(MosaicId id) {
				return mappers::ToInt64(id);
			}

			static auto MapToMongoDocument(const ModelType& descriptor) {
				return plugins::ToDbModel(descriptor);
			}

			static auto MapToMongoModels(const state::MosaicHistory& history, model::NetworkIdentifier) {
				return MosaicDescriptorsFromHistory(history);
			}

			static auto LoadSortOrder() {
				return document() << "mosaic.namespaceId" << 1 << "mosaic.mosaicId" << 1 << "meta.index" << 1 << finalize;
			}

			static void Insert(CacheDeltaType& cache, const bsoncxx::document::view& document) {
				cache.insert(*ToMosaicDescriptor(document).pEntry);
			}
		};
	}

	DEFINE_MONGO_HISTORICAL_CACHE_STORAGE(Mosaic, MosaicCacheTraits)
}}}
