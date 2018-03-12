#include "MongoTransactionStatusStorage.h"
#include "MongoBulkWriter.h"
#include "mappers/MapperUtils.h"
#include "mappers/TransactionStatusMapper.h"
#include "catapult/model/TransactionStatus.h"
#include "catapult/utils/SpinLock.h"
#include <mongocxx/client.hpp>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

	namespace {
		constexpr auto Collection_Name = "transactionStatuses";

		auto CreateFilter(const std::string& fieldName) {
			return [fieldName](const auto& status) {
				auto filter = document()
						<< fieldName
						<< open_document
							<< "$eq" << mongo::mappers::ToBinary(status.Hash)
						<< close_document
						<< finalize;
				return filter;
			};
		}

		class MongoTransactionStatusStorage final : public subscribers::TransactionStatusSubscriber {
		public:
			MongoTransactionStatusStorage(MongoStorageContext& context)
					: m_context(context)
					, m_database(m_context.createDatabaseConnection())
			{}

		public:
			void notifyStatus(const model::Transaction& transaction, const Hash256& hash, uint32_t status) override {
				utils::SpinLockGuard guard(m_lock);
				m_transactionStatuses.emplace_back(hash, status, transaction.Deadline);
			}

			void flush() override {
				decltype(m_transactionStatuses) transactionStatuses;
				{
					utils::SpinLockGuard guard(m_lock);
					transactionStatuses = std::move(m_transactionStatuses);
				}

				if (transactionStatuses.empty())
					return;

				// upsert into transaction statuses collection
				auto results = m_context.bulkWriter().bulkUpsert(
						Collection_Name,
						transactionStatuses,
						[](const auto& status, auto) { return mappers::ToDbModel(status); },
						CreateFilter("hash")).get();
				auto aggregateUpsert = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
				auto numTotalModified = mappers::ToUint32(aggregateUpsert.NumModified) + mappers::ToUint32(aggregateUpsert.NumUpserted);
				if (transactionStatuses.size() != numTotalModified) {
					CATAPULT_THROW_RUNTIME_ERROR_2(
							"could not save transaction statuses to transaction statuses collection (expected, actual)",
							transactionStatuses.size(),
							numTotalModified);
				}
			}

		private:
			MongoStorageContext& m_context;
			MongoDatabase m_database;
			std::vector<model::TransactionStatus> m_transactionStatuses;
			utils::SpinLock m_lock;
		};
	}

	std::unique_ptr<subscribers::TransactionStatusSubscriber> CreateMongoTransactionStatusStorage(MongoStorageContext& context) {
		return std::make_unique<MongoTransactionStatusStorage>(context);
	}
}}
