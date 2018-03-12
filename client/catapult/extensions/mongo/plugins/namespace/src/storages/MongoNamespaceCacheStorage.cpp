#include "MongoNamespaceCacheStorage.h"
#include "CacheStorageUtils.h"
#include "src/mappers/NamespaceDescriptorMapper.h"
#include "mongo/src/storages/MongoCacheStorage.h"
#include "plugins/txes/namespace/src/cache/NamespaceCache.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct NamespaceCacheTraits {
			using CacheType = cache::NamespaceCache;
			using CacheDeltaType = cache::NamespaceCacheDelta;
			using KeyType = NamespaceId;
			using ModelType = NamespaceDescriptor;
			using IdContainerType = std::unordered_set<KeyType, utils::BaseValueHasher<KeyType>>;
			using ElementContainerType = std::unordered_set<const state::RootNamespaceHistory*>;

			static constexpr const char* Collection_Name = "namespaces";
			static constexpr const char* Id_Property_Name = "namespace.level0";

			static auto GetId(const state::RootNamespaceHistory& history) {
				return history.id();
			}

			static auto MapToMongoId(NamespaceId id) {
				return mappers::ToInt64(id);
			}

			static auto MapToMongoDocument(const ModelType& descriptor) {
				return plugins::ToDbModel(descriptor);
			}

			static auto MapToMongoModels(const state::RootNamespaceHistory& history, model::NetworkIdentifier networkIdentifier) {
				return NamespaceDescriptorsFromHistory(history, networkIdentifier);
			}

			static auto LoadSortOrder() {
				return document() << "namespace.level0" << 1 << "meta.index" << 1 << "namespace.depth" << 1 << finalize;
			}

			static void Insert(CacheDeltaType& cache, const bsoncxx::document::view& document) {
				auto descriptor = ToNamespaceDescriptor(document);
				if (descriptor.IsRoot()) {
					cache.insert(*descriptor.pRoot);
				} else {
					// it might be an inherited child
					auto ns = state::Namespace(descriptor.Path);
					if (!cache.contains(ns.id()))
						cache.insert(ns);
				}
			}
		};
	}

	DEFINE_MONGO_HISTORICAL_CACHE_STORAGE(Namespace, NamespaceCacheTraits)
}}}
