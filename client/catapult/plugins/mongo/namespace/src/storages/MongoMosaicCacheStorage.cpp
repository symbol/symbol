#include "MongoMosaicCacheStorage.h"
#include "CacheStorageUtils.h"
#include "plugins/mongo/coremongo/src/MongoBulkWriter.h"
#include "plugins/mongo/coremongo/src/MongoStorageConfiguration.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/mongo/namespace/src/mappers/MosaicDescriptorMapper.h"
#include "plugins/txes/namespace/src/cache/MosaicCache.h"
#include "catapult/thread/FutureUtils.h"

using namespace bsoncxx::builder::stream;
using namespace catapult::mongo::plugins;

namespace catapult { namespace mongo { namespace storages {

	namespace {
		std::vector<MosaicId> CollectMosaicIds(const std::vector<const state::MosaicHistory*>& histories) {
			std::vector<MosaicId> ids;
			for (const auto* pHistory : histories)
				ids.push_back(pHistory->id());

			return ids;
		}

		bsoncxx::document::value CreateDeleteFilter(const std::vector<MosaicId>& ids) {
			if (ids.empty())
				return document() << finalize;

			document doc{};
			auto array = doc
					<< "mosaic.mosaicId"
					<< open_document
						<< "$in"
						<< open_array;

			for (auto id : ids)
				array << mappers::ToInt64(id);

			array << close_array;
			doc << close_document;
			return doc << finalize;
		}

		class MongoMosaicCacheStorage : public ExternalCacheStorageT<cache::MosaicCache> {
		public:
			explicit MongoMosaicCacheStorage(MongoDatabase&& database, MongoBulkWriter& bulkWriter)
					: m_database(std::move(database))
					, m_bulkWriter(bulkWriter)
			{}

		private:
			void saveDelta(const cache::MosaicCacheDelta& cache) override {
				saveMosaics(
						cache.addedMosaicHistories(),
						cache.modifiedMosaicHistories(),
						cache.removedMosaicHistories());
			}

		private:
			void saveMosaics(
					const std::vector<const state::MosaicHistory*>& addedHistories,
					const std::vector<const state::MosaicHistory*>& modifiedHistories,
					const std::vector<MosaicId>& removedHistories) {
				auto mosaics = m_database["mosaics"];

				// - remove all modified and removed histories
				auto ids = CollectMosaicIds(modifiedHistories);
				ids.insert(ids.cend(), removedHistories.cbegin(), removedHistories.cend());
				if (!ids.empty()) {
					auto filter = CreateDeleteFilter(ids);
					auto deleteResult = mosaics.delete_many(filter.view());
					if (!deleteResult || ids.size() > mappers::ToUint32(deleteResult.get().deleted_count()))
						CATAPULT_THROW_RUNTIME_ERROR("could not delete modified and removed mosaic histories");
				}

				// - insert new histories
				auto createDocument = [](const auto& mosaicDescriptor, auto) {
					return mappers::ToDbModel(mosaicDescriptor);
				};

				std::vector<const state::MosaicHistory*> newHistories(addedHistories);
				newHistories.insert(newHistories.cbegin(), modifiedHistories.cbegin(), modifiedHistories.cend());
				std::vector<state::MosaicDescriptor> allDescriptors;
				for (const auto* pHistory : newHistories) {
					auto descriptors = MosaicDescriptorsFromHistory(*pHistory);
					std::move(descriptors.begin(), descriptors.end(), std::back_inserter(allDescriptors));
				}

				auto insertResult = m_bulkWriter.bulkInsert("mosaics", allDescriptors, createDocument).get();
				auto aggregateInsert = BulkWriteResult::Aggregate(thread::get_all(std::move(insertResult)));
				if (allDescriptors.size() != mappers::ToUint32(aggregateInsert.NumInserted))
					CATAPULT_THROW_RUNTIME_ERROR("could not insert modified mosaic histories");
			}

		private:
			void loadAll(cache::MosaicCacheDelta& cache, Height, const LoadCheckpointFunc& checkpoint) const override {
				auto counter = 0u;
				auto mosaics = m_database["mosaics"];
				mongocxx::options::find options;
				auto sortOrder = document{}
						<< "mosaic.namespaceId" << 1
						<< "mosaic.mosaicId" << 1
						<< "meta.index" << 1
						<< finalize;
				options.sort(sortOrder.view());
				auto cursor = mosaics.find({}, options);
				for (const auto& element : cursor) {
					if (0 == ++counter % 10'000) {
						checkpoint();
						CATAPULT_LOG(info) << "committing " << counter << " mosaics";
					}

					auto descriptor = mappers::ToMosaicDescriptor(element);
					cache.insert(*descriptor.pEntry);
				}
			}

		private:
			MongoDatabase m_database;
			MongoBulkWriter& m_bulkWriter;
		};
	}

	std::unique_ptr<ExternalCacheStorage> CreateMongoMosaicCacheStorage(MongoDatabase&& database, MongoBulkWriter& bulkWriter) {
		return std::make_unique<MongoMosaicCacheStorage>(std::move(database), bulkWriter);
	}
}}}
