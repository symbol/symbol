#include "MongoBlockStorage.h"
#include "MongoBulkWriter.h"
#include "MongoChainInfoUtils.h"
#include "MongoTransactionMetadata.h"
#include "mappers/BlockMapper.h"
#include "mappers/HashMapper.h"
#include "mappers/MapperUtils.h"
#include "mappers/TransactionMapper.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

	namespace {
		void SetHeight(mongocxx::database& database, Height height) {
			auto journalHeight = document()
					<< "$set"
					<< open_document << "height" << static_cast<int64_t>(height.unwrap()) << close_document
					<< finalize;

			SetChainInfoDocument(database, journalHeight.view());
		}

		model::HashRange LoadHashes(const mongocxx::database& database, Height height, size_t numHashes) {
			auto blocks = database["blocks"];
			auto filter = document()
					<< "block.height"
					<< open_document
						<< "$gte" << static_cast<int64_t>(height.unwrap())
						<< "$lt" << static_cast<int64_t>(height.unwrap() + numHashes)
					<< close_document
					<< finalize;

			mongocxx::options::find options;
			options.sort(document() << "block.height" << 1 << finalize);
			options.projection(document() << "meta.hash" << 1 << finalize);

			auto cursor = blocks.find(filter.view(), options);
			return mappers::ToModel(cursor, numHashes);
		}

		void HandleDropResult(const bsoncxx::stdx::optional<mongocxx::result::delete_result>& result, const char* collectionName) {
			if (result)
				CATAPULT_LOG(info) << "deleted " << result->deleted_count() << " " << collectionName;
			else
				CATAPULT_THROW_RUNTIME_ERROR("delete returned empty result");
		}

		void DropBlocks(mongocxx::database& database, Height height) {
			auto blocks = database["blocks"];
			auto filter = document()
					<< "block.height"
					<< open_document << "$gt" << static_cast<int64_t>(height.unwrap()) << close_document
					<< finalize;
			auto result = blocks.delete_many(filter.view());
			HandleDropResult(result, "blocks");
		}

		void DropTransactions(mongocxx::database& database, Height height) {
			auto transactions = database["transactions"];
			auto filter = document()
					<< "meta.height"
					<< open_document << "$gt" << static_cast<int64_t>(height.unwrap()) << close_document
					<< finalize;
			auto result = transactions.delete_many(filter.view());
			HandleDropResult(result, "transactions");
		}

		class MongoBlockStorage final : public io::LightBlockStorage {
		public:
			MongoBlockStorage(MongoStorageContext& context, const MongoTransactionRegistry& transactionRegistry)
					: m_context(context)
					, m_transactionRegistry(transactionRegistry)
					, m_database(m_context.createDatabaseConnection())
			{}

		public:
			Height chainHeight() const override {
				auto chainInfoDocument = GetChainInfoDocument(m_database);
				if (mappers::IsEmptyDocument(chainInfoDocument))
					return Height();

				auto heightValue = mappers::GetUint64OrDefault(chainInfoDocument.view(), "height", 0);
				return Height(heightValue);
			}

		public:
			model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override {
				auto dbHeight = chainHeight();
				if (Height(0) == height || dbHeight < height)
					return model::HashRange();

				auto numAvailableBlocks = static_cast<size_t>((dbHeight - height).unwrap() + 1);
				auto numHashes = std::min(maxHashes, numAvailableBlocks);

				return LoadHashes(m_database, height, numHashes);
			}

			void saveBlock(const model::BlockElement& blockElement) override {
				auto dbHeight = chainHeight();
				auto height = blockElement.Block.Height;

				if (height != dbHeight + Height(1))
					CATAPULT_THROW_INVALID_ARGUMENT_2("cannot save out of order block (block height, chain height)", height, dbHeight);

				auto blocks = m_database["blocks"];

				auto dbBlock = mappers::ToDbModel(blockElement);
				auto result = blocks.insert_one(dbBlock.view()).get().result();
				if (0 == result.inserted_count())
					CATAPULT_THROW_RUNTIME_ERROR("saveBlock failed: block header was not inserted");

				const auto& registry = m_transactionRegistry;
				std::atomic<size_t> numTotalTransactionDocuments(0);
				auto createDocuments = [height, &registry, &numTotalTransactionDocuments](const auto& transactionElement, auto index) {
					auto metadata = MongoTransactionMetadata(transactionElement, height, index);
					auto documents = mappers::ToDbDocuments(transactionElement.Transaction, metadata, registry);
					numTotalTransactionDocuments += documents.size();
					return documents;
				};
				auto results = m_context.bulkWriter().bulkInsert("transactions", blockElement.Transactions, createDocuments).get();
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

			void dropBlocksAfter(Height height) override {
				auto dbHeight = chainHeight();
				if (dbHeight <= height)
					return;

				SetHeight(m_database, height);

				DropBlocks(m_database, height);
				DropTransactions(m_database, height);
			}

		private:
			MongoStorageContext& m_context;
			const MongoTransactionRegistry& m_transactionRegistry;
			MongoDatabase m_database;
		};
	}

	std::unique_ptr<io::LightBlockStorage> CreateMongoBlockStorage(
			MongoStorageContext& context,
			const MongoTransactionRegistry& transactionRegistry) {
		return std::make_unique<MongoBlockStorage>(context, transactionRegistry);
	}
}}
