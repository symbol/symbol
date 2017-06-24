#include "MongoMultisigCacheStorage.h"
#include "plugins/mongo/coremongo/src/MongoBulkWriter.h"
#include "plugins/mongo/coremongo/src/MongoStorageConfiguration.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/mongo/multisig/src/mappers/MultisigEntryMapper.h"
#include "plugins/txes/multisig/src/cache/MultisigCache.h"
#include "catapult/thread/FutureUtils.h"

using namespace bsoncxx::builder::stream;
using namespace catapult::mongo::plugins;

namespace catapult { namespace mongo { namespace storages {

	namespace {
		bsoncxx::document::value CreateDeleteFilter(const std::vector<Key>& keys) {
			if (keys.empty())
				return document() << finalize;

			document doc{};
			auto array = doc
					<< "multisig.account"
					<< open_document
						<< "$in"
						<< open_array;

			for (const auto& key : keys)
				array << mappers::ToBinary(key);

			array << close_array;
			doc << close_document;
			return doc << finalize;
		}

		class MongoMultisigCacheStorage : public ExternalCacheStorageT<cache::MultisigCache> {
		public:
			explicit MongoMultisigCacheStorage(MongoDatabase&& database, MongoBulkWriter& bulkWriter)
					: m_database(std::move(database))
					, m_bulkWriter(bulkWriter)
			{}

		private:
			void saveDelta(const cache::MultisigCacheDelta& cache) override {
				saveMultisigEntries(cache.modifiedEntries(), cache.removedEntries());
			}

		private:
			void saveMultisigEntries(
					const std::vector<const state::MultisigEntry*>& modifiedEntries,
					const std::vector<Key>& removedEntries) {
				auto multisigs = m_database["multisigs"];

				// - remove all removed entries
				if (!removedEntries.empty()) {
					auto filter = CreateDeleteFilter(removedEntries);
					auto deleteResult = multisigs.delete_many(filter.view());
					if (!deleteResult || removedEntries.size() > mappers::ToUint32(deleteResult.get().deleted_count()))
						CATAPULT_THROW_RUNTIME_ERROR("could not delete removed multisig entries");
				}

				// - upsert modified entries
				auto createDocument = [](const auto* pMultisigEntry, auto) {
					return mappers::ToDbModel(*pMultisigEntry);
				};

				auto filterDocument = [](const auto* pMultisigEntry) {
					return document()
							<< "multisig.account" << mappers::ToBinary(pMultisigEntry->key())
							<< finalize;
				};

				auto upsertResult = m_bulkWriter.bulkUpsert("multisigs", modifiedEntries, createDocument, filterDocument).get();
				auto aggregateUpsert = BulkWriteResult::Aggregate(thread::get_all(std::move(upsertResult)));
				auto numTotalModified = mappers::ToUint32(aggregateUpsert.NumModified) + mappers::ToUint32(aggregateUpsert.NumUpserted);
				if (modifiedEntries.size() != numTotalModified)
					CATAPULT_THROW_RUNTIME_ERROR("could not upsert modified multisig entries");
			}

		private:
			void loadAll(cache::MultisigCacheDelta& cache, Height, const LoadCheckpointFunc& checkpoint) const override {
				auto counter = 0u;
				auto multisigs = m_database["multisigs"];
				auto cursor = multisigs.find({});
				for (const auto& element : cursor) {
					if (0 == ++counter % 10'000) {
						checkpoint();
						CATAPULT_LOG(info) << "committing " << counter << " multsig entries";
					}

					auto entry = mappers::ToMultisigEntry(element);
					cache.insert(entry);
				}
			}

		private:
			MongoDatabase m_database;
			MongoBulkWriter& m_bulkWriter;
		};
	}

	std::unique_ptr<ExternalCacheStorage> CreateMongoMultisigCacheStorage(MongoDatabase&& database, MongoBulkWriter& bulkWriter) {
		return std::make_unique<MongoMultisigCacheStorage>(std::move(database), bulkWriter);
	}
}}}
