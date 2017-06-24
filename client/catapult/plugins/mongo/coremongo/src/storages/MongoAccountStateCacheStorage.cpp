#include "MongoAccountStateCacheStorage.h"
#include "src/MongoBulkWriter.h"
#include "src/MongoDatabase.h"
#include "src/MongoStorageConfiguration.h"
#include "src/mappers/AccountStateMapper.h"
#include "src/mappers/MapperUtils.h"
#include "catapult/cache/AccountStateCache.h"

using namespace bsoncxx::builder::stream;
using namespace catapult::mongo::plugins;

namespace catapult { namespace mongo { namespace storages {

	namespace {
		using AccountStates = std::unordered_set<std::shared_ptr<const state::AccountState>>;

		class MongoAccountStateCacheStorage : public ExternalCacheStorageT<cache::AccountStateCache> {
		public:
			explicit MongoAccountStateCacheStorage(MongoDatabase&& database, MongoBulkWriter& bulkWriter)
					: m_database(std::move(database))
					, m_bulkWriter(bulkWriter)
			{}

		private:
			void saveDelta(const cache::AccountStateCacheDelta& cache) {
				saveAccountStates(cache.modifiedAccountStates(), cache.removedAccountStates());
			}

		private:
			void saveAccountStates(const AccountStates& modifiedAccountStates, const AccountStates& removedAccountStates) {
				auto createDocument = [](const auto& pAccountState, auto) {
					return mappers::ToDbModel(*pAccountState);
				};

				auto filterDocument = [](const auto& pAccountState) {
					return document()
							<< "account.address"
							<< open_document
								<< "$eq" << mappers::ToBinary(pAccountState->Address)
							<< close_document
							<< finalize;
				};

				auto upsertResults = m_bulkWriter.bulkUpsert(
						"accounts",
						modifiedAccountStates,
						createDocument,
						filterDocument).get();
				auto aggregateUpsert = BulkWriteResult::Aggregate(thread::get_all(std::move(upsertResults)));
				auto numTotalModified = mappers::ToUint32(aggregateUpsert.NumModified) + mappers::ToUint32(aggregateUpsert.NumUpserted);
				if (modifiedAccountStates.size() != numTotalModified)
					CATAPULT_THROW_RUNTIME_ERROR("could not save account states");

				auto deleteResults = m_bulkWriter.bulkDelete("accounts", removedAccountStates, filterDocument).get();
				auto aggregateDelete = BulkWriteResult::Aggregate(thread::get_all(std::move(deleteResults)));
				if (removedAccountStates.size() != mappers::ToUint32(aggregateDelete.NumDeleted))
					CATAPULT_THROW_RUNTIME_ERROR("could not delete account states");
			}

		private:
			void loadAll(cache::AccountStateCacheDelta& cache, Height, const LoadCheckpointFunc& checkpoint) const {
				auto counter = 0u;
				auto accounts = m_database["accounts"];
				auto cursor = accounts.find({});
				for (const auto& element : cursor) {
					if (0 == ++counter % 10'000) {
						checkpoint();
						CATAPULT_LOG(info) << "committing " << counter << " accounts";
					}

					mappers::ToAccountState(element, [&cache](const Address& address, Height height) {
						return cache.addAccount(address, height);
					});
				}
			}

		private:
			MongoDatabase m_database;
			MongoBulkWriter& m_bulkWriter;
		};
	}

	std::unique_ptr<ExternalCacheStorage> CreateMongoAccountStateCacheStorage(MongoDatabase&& database, MongoBulkWriter& bulkWriter) {
		return std::make_unique<MongoAccountStateCacheStorage>(std::move(database), bulkWriter);
	}
}}}
