#include "MongoDbStorage.h"
#include "MongoBulkWriter.h"
#include "MongoTransactionMetadata.h"
#include "mappers/AccountStateMapper.h"
#include "mappers/BlockMapper.h"
#include "mappers/HashMapper.h"
#include "mappers/MapperUtils.h"
#include "mappers/TransactionMapper.h"
#include "catapult/cache/BlockDifficultyCache.h"
#include "catapult/model/ChainScore.h"
#include "catapult/thread/FutureUtils.h"
#include <boost/optional.hpp>
#include <mongocxx/cursor.hpp>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		void SetChainInfoDocument(mongocxx::database& database, const bsoncxx::document::view& upsertDoc) {
			auto dbState = database["chainInfo"];
			auto result = dbState.update_one({}, upsertDoc, mongocxx::options::update().upsert(true)).get().result();
			if (0 == result.modified_count() && 0 == result.upserted_count())
				CATAPULT_THROW_RUNTIME_ERROR("SetChainInfoDocument failed: no document upserted or modified");
		}

		void SetHeight(mongocxx::database& database, Height height) {
			auto journalHeight = document{}
					<< "$set"
					<< open_document << "height" << static_cast<int64_t>(height.unwrap()) << close_document
					<< finalize;

			SetChainInfoDocument(database, journalHeight.view());
		}

		void SetScore(mongocxx::database& database, const model::ChainScore& score) {
			auto scoreArray = score.toArray();
			auto scoreValue = document{}
					<< "$set"
					<< open_document
						<< "scoreHigh" << static_cast<int64_t>(scoreArray[0])
						<< "scoreLow" << static_cast<int64_t>(scoreArray[1])
					<< close_document
					<< finalize;

			SetChainInfoDocument(database, scoreValue.view());
		}

		void EmptyDeleter(uint8_t*) {}

		bsoncxx::document::value GetChainInfoDocument(const mongocxx::database& database) {
			auto dbState = database["chainInfo"];
			auto matchedDocument = dbState.find_one({});
			if (matchedDocument.is_initialized())
				return matchedDocument.get();

			return bsoncxx::document::value(nullptr, 0, EmptyDeleter);
		}

		uint64_t GetUint64OrDefault(const bsoncxx::document::view& documentView, const char* name, uint64_t defaultValue) {
			auto it = documentView.find(name);
			if (documentView.end() == it)
				return defaultValue;

			return static_cast<uint64_t>(it->get_int64().value);
		}

		bool IsEmptyDocument(const bsoncxx::document::value& document) {
			// note: can't check .empty() here...
			return 0 == document.view().length();
		}
	}

	MongoDbStorage::MongoDbStorage(
			const std::shared_ptr<MongoStorageConfiguration>& pConfig,
			const std::shared_ptr<const MongoTransactionRegistry>& pTransactionRegistry)
			: m_pConfig(pConfig)
			, m_pTransactionRegistry(pTransactionRegistry)
			, m_database(m_pConfig->createDatabaseConnection())
	{}

	Height MongoDbStorage::chainHeight() const {
		auto chainInfoDocument = GetChainInfoDocument(m_database);
		if (IsEmptyDocument(chainInfoDocument))
			return Height();

		auto heightValue = GetUint64OrDefault(chainInfoDocument.view(), "height", 0u);
		return Height(heightValue);
	}

	namespace {
		model::HashRange LoadHashes(const mongocxx::database& database, Height height, size_t numHashes) {
			auto blocks = database["blocks"];
			auto filter = document{}
					<< "block.height"
					<< open_document
						<< "$gte" << static_cast<int64_t>(height.unwrap())
						<< "$lt" << static_cast<int64_t>(height.unwrap() + numHashes)
					<< close_document
					<< finalize;

			mongocxx::options::find options;
			options.sort(document{} << "block.height" << 1 << finalize);
			options.projection(document{} << "meta.hash" << 1 << finalize);

			auto cursor = blocks.find(filter.view(), options);
			return mappers::ToModel(cursor, numHashes);
		}
	}

	model::HashRange MongoDbStorage::loadHashesFrom(Height height, size_t maxHashes) const {
		auto currentHeight = chainHeight();
		if (Height(0) == height || currentHeight < height)
			return model::HashRange();

		auto numAvailableBlocks = static_cast<size_t>((currentHeight - height).unwrap() + 1);
		auto numHashes = std::min(maxHashes, numAvailableBlocks);

		return LoadHashes(m_database, height, numHashes);
	}

	void MongoDbStorage::saveBlock(const model::BlockElement& blockElement) {
		auto currentHeight = chainHeight();
		auto height = blockElement.Block.Height;

		if (height != currentHeight + Height(1))
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot save out of order block at height", height);

		auto blocks = m_database["blocks"];

		auto dbBlock = mappers::ToDbModel(blockElement);
		auto result = blocks.insert_one(dbBlock.view()).get().result();
		if (0 == result.inserted_count())
			CATAPULT_THROW_RUNTIME_ERROR("saveBlock failed: block header was not inserted");

		const auto& registry = *m_pTransactionRegistry;
		std::atomic<size_t> numTotalTransactionDocuments(0);
		auto createDocuments = [height, &registry, &numTotalTransactionDocuments](const auto& txElement, auto index) {
			auto metadata = MongoTransactionMetadata(txElement.EntityHash, txElement.MerkleComponentHash, height, index);
			auto documents = mappers::ToDbDocuments(txElement.Transaction, metadata, registry);
			numTotalTransactionDocuments += documents.size();
			return documents;
		};
		auto results = m_pConfig->bulkWriter().bulkInsert("transactions", blockElement.Transactions, createDocuments).get();
		auto aggregate = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
		auto numInsertedDocuments = mappers::ToUint32(aggregate.NumInserted);
		if (blockElement.Transactions.size() > numInsertedDocuments || numTotalTransactionDocuments != numInsertedDocuments) {
			CATAPULT_LOG(error)
					<< "only inserted " << numInsertedDocuments << " documents with "
					<< blockElement.Transactions.size() << " transactions and "
					<< numTotalTransactionDocuments << " expected documents at height " << height;
			CATAPULT_THROW_RUNTIME_ERROR_1("could not insert transactions for block at height", height);
		}

		SetHeight(m_database, blockElement.Block.Height);
	}

	namespace {
		void HandleDropResult(const bsoncxx::stdx::optional<mongocxx::result::delete_result>& result, const char* collectionName) {
			if (result)
				CATAPULT_LOG(info) << "deleted " << result->deleted_count() << " " << collectionName;
			else
				CATAPULT_THROW_RUNTIME_ERROR("delete returned empty result");
		}

		void DropBlocks(mongocxx::database& database, Height height) {
			auto blocks = database["blocks"];
			auto filter = document{}
					<< "block.height"
					<< open_document << "$gt" << static_cast<int64_t>(height.unwrap()) << close_document
					<< finalize;
			auto result = blocks.delete_many(filter.view());
			HandleDropResult(result, "blocks");
		}

		void DropTransactions(mongocxx::database& database, Height height) {
			auto transactions = database["transactions"];
			auto filter = document{}
					<< "meta.height"
					<< open_document << "$gt" << static_cast<int64_t>(height.unwrap()) << close_document
					<< finalize;
			auto result = transactions.delete_many(filter.view());
			HandleDropResult(result, "transactions");
		}
	}

	void MongoDbStorage::dropBlocksAfter(Height height) {
		auto currentHeight = chainHeight();
		if (currentHeight <= height)
			return;

		SetHeight(m_database, height);

		DropBlocks(m_database, height);
		DropTransactions(m_database, height);
	}

	void MongoDbStorage::saveScore(const model::ChainScore& chainScore) {
		SetScore(m_database, chainScore);
	}

	model::ChainScore MongoDbStorage::loadScore() const {
		auto chainInfoDocument = GetChainInfoDocument(m_database);
		if (IsEmptyDocument(chainInfoDocument))
			return model::ChainScore();

		auto scoreLow = GetUint64OrDefault(chainInfoDocument.view(), "scoreLow", 0u);
		auto scoreHigh = GetUint64OrDefault(chainInfoDocument.view(), "scoreHigh", 0u);
		return model::ChainScore(scoreHigh, scoreLow);
	}
}}}
