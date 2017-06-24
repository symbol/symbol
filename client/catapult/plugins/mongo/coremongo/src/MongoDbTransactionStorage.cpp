#include "MongoDbTransactionStorage.h"
#include "MongoBulkWriter.h"
#include "MongoTransactionMetadata.h"
#include "mappers/MapperUtils.h"
#include "mappers/TransactionMapper.h"
#include <boost/optional.hpp>
#include <mongocxx/cursor.hpp>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		constexpr auto Ut_Collection_Name = "unconfirmedTransactions";
	}

	MongoDbTransactionStorage::MongoDbTransactionStorage(
			const std::shared_ptr<MongoStorageConfiguration>& pConfig,
			const std::shared_ptr<const MongoTransactionRegistry>& pTransactionRegistry)
			: m_pConfig(pConfig)
			, m_pTransactionRegistry(pTransactionRegistry)
			, m_database(m_pConfig->createDatabaseConnection())
	{}

	// region transaction storage

	bool MongoDbTransactionStorage::saveTransaction(const model::TransactionInfo& info) {
		// if the hash can be found in the set of deleted transactions, it means that
		// it is currently in the collection so removing the hash from the set means (re)adding it
		auto iter = m_deletedTransactionHashes.find(info.EntityHash);
		if (m_deletedTransactionHashes.cend() != iter)
			m_deletedTransactionHashes.erase(iter);
		else
			m_addedTransactions.emplace(info.copy());

		return true;
	}

	void MongoDbTransactionStorage::removeTransaction(const Hash256& hash) {
		// if the hash can be found in the set of added transactions, simply remove it
		// this only works because transactions are immutable (there are no upserts)
		// so a newly saved transaction cannot already be in the database
		model::TransactionInfo searchInfo;
		searchInfo.EntityHash = hash;
		auto iter = m_addedTransactions.find(searchInfo);

		if (m_addedTransactions.cend() != iter)
			m_addedTransactions.erase(iter);
		else
			m_deletedTransactionHashes.insert(hash);
	}

	void MongoDbTransactionStorage::removeTransactions(const std::vector<model::TransactionInfo>& transactionInfos) {
		for (const auto& info : transactionInfos)
			removeTransaction(info.EntityHash);
	}

	void MongoDbTransactionStorage::pruneTransactions(Timestamp timestamp) {
		auto unconfirmedTransactions = m_database[Ut_Collection_Name];
		auto filter = document{}
				<< "transaction.deadline"
				<< open_document << "$lt" << static_cast<int64_t>(timestamp.unwrap()) << close_document
				<< finalize;
		unconfirmedTransactions.delete_many(filter.view());
	}

	void MongoDbTransactionStorage::commit() {
		commitDeletes();
		commitInserts();
	}

	namespace {
		auto GetObjectIdsFilter(const std::unordered_set<Hash256, utils::ArrayHasher<Hash256>>& hashes) {
			document doc{};
			auto array = doc
					<< "meta.hash"
					<< open_document
						<< "$in"
						<< open_array;

			for (const auto& hash : hashes)
				array << mappers::ToBinary(hash);

			array << close_array;
			doc << close_document;
			return doc << finalize;
		}

		auto GetTransactionObjectIds(
				const mongocxx::database& database,
				const std::unordered_set<Hash256, utils::ArrayHasher<Hash256>>& hashes) {
			mongocxx::options::find options;
			options.projection(document{} << "_id" << 1 << finalize);

			auto cursor = database[Ut_Collection_Name].find(GetObjectIdsFilter(hashes), options);
			std::vector<bsoncxx::oid> objectIds;
			for (const auto& element : cursor)
				objectIds.push_back(element["_id"].get_oid().value);

			return objectIds;
		}
	}

	void MongoDbTransactionStorage::commitDeletes() {
		if (m_deletedTransactionHashes.empty())
			return;

		auto objectIds = GetTransactionObjectIds(m_database, m_deletedTransactionHashes);

		auto createFilter = [](const auto& objectId) {
			return document{}
					<< "$or"
						<< open_array
							<< open_document << "_id" << objectId << close_document
							<< open_document << "meta.aggregateId" << objectId << close_document
						<< close_array
					<< finalize;
		};
		auto results = m_pConfig->bulkWriter().bulkDelete(Ut_Collection_Name, objectIds, createFilter).get();
		auto aggregate = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		auto numDeleted = mappers::ToUint32(aggregate.NumDeleted);
		if (m_deletedTransactionHashes.size() > numDeleted) {
			CATAPULT_THROW_RUNTIME_ERROR_2(
					"delete unconfirmed transactions failed: delete count mismatch (min expected, actual)",
					m_deletedTransactionHashes.size(),
					numDeleted);
		}

		m_deletedTransactionHashes.clear();
	}

	void MongoDbTransactionStorage::commitInserts() {
		if (m_addedTransactions.empty())
			return;

		// note that unconfirmed transactions don't have height and block index metadata
		const auto& registry = *m_pTransactionRegistry;
		std::atomic<size_t> numTotalTransactionDocuments(0);
		auto createDocuments = [&registry, &numTotalTransactionDocuments](const auto& transactionInfo, auto) {
			auto metadata = MongoTransactionMetadata(transactionInfo.EntityHash, transactionInfo.MerkleComponentHash);
			auto documents = mappers::ToDbDocuments(*transactionInfo.pEntity, metadata, registry);
			numTotalTransactionDocuments += documents.size();
			return documents;
		};
		auto results = m_pConfig->bulkWriter().bulkInsert(Ut_Collection_Name, m_addedTransactions, createDocuments).get();
		auto aggregate = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		auto numInsertedDocuments = mappers::ToUint32(aggregate.NumInserted);
		if (m_addedTransactions.size() > numInsertedDocuments || numTotalTransactionDocuments != numInsertedDocuments) {
			CATAPULT_THROW_RUNTIME_ERROR_2(
					"insert unconfirmed transactions failed: insert count mismatch (expected, actual)",
					m_addedTransactions.size(),
					numInsertedDocuments);
		}

		m_addedTransactions.clear();
	}

	// endregion
}}}
