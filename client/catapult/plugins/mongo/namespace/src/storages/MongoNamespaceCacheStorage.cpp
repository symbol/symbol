#include "MongoNamespaceCacheStorage.h"
#include "CacheStorageUtils.h"
#include "plugins/mongo/coremongo/src/MongoBulkWriter.h"
#include "plugins/mongo/coremongo/src/MongoDatabase.h"
#include "plugins/mongo/coremongo/src/MongoStorageConfiguration.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/mongo/namespace/src/mappers/NamespaceDescriptorMapper.h"
#include "plugins/txes/namespace/src/cache/NamespaceCache.h"

using namespace bsoncxx::builder::stream;
using namespace catapult::mongo::plugins;

namespace catapult { namespace mongo { namespace storages {

	namespace {
		std::vector<NamespaceId> CollectNamespaceIds(const std::vector<const state::RootNamespaceHistory*>& histories) {
			std::vector<NamespaceId> ids;
			for (const auto* pHistory : histories)
				ids.push_back(pHistory->id());

			return ids;
		}

		bsoncxx::document::value CreateDeleteFilter(const std::vector<NamespaceId>& ids) {
			if (ids.empty())
				return document() << finalize;

			document doc{};
			auto array = doc
					<< "namespace.level0"
					<< open_document
						<< "$in"
						<< open_array;

			for (auto id : ids)
				array << mappers::ToInt64(id);

			array << close_array;
			doc << close_document;
			return doc << finalize;
		}

		class MongoNamespaceCacheStorage : public ExternalCacheStorageT<cache::NamespaceCache> {
		public:
			explicit MongoNamespaceCacheStorage(MongoDatabase&& database, MongoBulkWriter& bulkWriter)
					: m_database(std::move(database))
					, m_bulkWriter(bulkWriter)
			{}

		private:
			void saveDelta(const cache::NamespaceCacheDelta& cache) override {
				saveNamespaces(
						cache.addedRootNamespaceHistories(),
						cache.modifiedRootNamespaceHistories(),
						cache.removedRootNamespaceHistories());
			}

		private:
			void saveNamespaces(
					const std::vector<const state::RootNamespaceHistory*>& addedHistories,
					const std::vector<const state::RootNamespaceHistory*>& modifiedHistories,
					const std::vector<NamespaceId>& removedHistories) {
				auto namespaces = m_database["namespaces"];

				// - remove all modified and removed histories
				auto ids = CollectNamespaceIds(modifiedHistories);
				ids.insert(ids.cend(), removedHistories.cbegin(), removedHistories.cend());
				if (!ids.empty()) {
					auto filter = CreateDeleteFilter(ids);
					auto deleteResult = namespaces.delete_many(filter.view());
					if (!deleteResult || ids.size() > mappers::ToUint32(deleteResult.get().deleted_count()))
						CATAPULT_THROW_RUNTIME_ERROR("could not delete modified and removed namespace histories");
				}

				// - insert new histories
				auto createDocument = [](const auto& namespaceDescriptor, auto) {
					return mappers::ToDbModel(namespaceDescriptor);
				};

				std::vector<const state::RootNamespaceHistory*> newHistories(addedHistories);
				newHistories.insert(newHistories.cbegin(), modifiedHistories.cbegin(), modifiedHistories.cend());
				std::vector<state::NamespaceDescriptor> allDescriptors;
				for (const auto* pHistory : newHistories) {
					auto descriptors = NamespaceDescriptorsFromHistory(*pHistory);
					std::move(descriptors.begin(), descriptors.end(), std::back_inserter(allDescriptors));
				}

				auto insertResult = m_bulkWriter.bulkInsert("namespaces", allDescriptors, createDocument).get();
				auto aggregateInsert = BulkWriteResult::Aggregate(thread::get_all(std::move(insertResult)));
				if (allDescriptors.size() != mappers::ToUint32(aggregateInsert.NumInserted))
					CATAPULT_THROW_RUNTIME_ERROR("could not insert modified namespace histories");
			}

		private:
			void loadAll(cache::NamespaceCacheDelta& cache, Height, const LoadCheckpointFunc& checkpoint) const override {
				auto counter = 0u;
				auto namespaces = m_database["namespaces"];
				mongocxx::options::find options;
				auto sortOrder = document{}
						<< "namespace.level0" << 1
						<< "meta.index" << 1
						<< "namespace.depth" << 1
						<< finalize;
				options.sort(sortOrder.view());
				auto cursor = namespaces.find({}, options);
				for (const auto& element : cursor) {
					if (0 == ++counter % 10'000) {
						checkpoint();
						CATAPULT_LOG(info) << "committing " << counter << " namespaces";
					}

					auto descriptor = mappers::ToNamespaceDescriptor(element);
					if (descriptor.IsRoot()) {
						cache.insert(*descriptor.pRoot);
					} else {
						// it might be an inherited child
						auto ns = state::Namespace(descriptor.Path);
						if (!cache.contains(ns.id()))
							cache.insert(ns);
					}
				}
			}

		private:
			MongoDatabase m_database;
			MongoBulkWriter& m_bulkWriter;
		};
	}

	std::unique_ptr<ExternalCacheStorage> CreateMongoNamespaceCacheStorage(MongoDatabase&& database, MongoBulkWriter& bulkWriter) {
		return std::make_unique<MongoNamespaceCacheStorage>(std::move(database), bulkWriter);
	}
}}}
