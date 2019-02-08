/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "MongoBlockStorage.h"
#include "MongoBulkWriter.h"
#include "MongoChainInfoUtils.h"
#include "MongoReceiptPlugin.h"
#include "MongoTransactionMetadata.h"
#include "mappers/BlockMapper.h"
#include "mappers/HashMapper.h"
#include "mappers/MapperUtils.h"
#include "mappers/ResolutionStatementMapper.h"
#include "mappers/TransactionMapper.h"
#include "mappers/TransactionStatementMapper.h"

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

		void SaveBlockHeader(const mongocxx::database& database, const model::BlockElement& blockElement) {
			auto blocks = database["blocks"];

			auto dbBlock = mappers::ToDbModel(blockElement);
			auto result = blocks.insert_one(dbBlock.view()).get().result();
			if (0 == result.inserted_count())
				CATAPULT_THROW_RUNTIME_ERROR("saveBlock failed: block header was not inserted");
		}

		void SaveTransactions(
				MongoBulkWriter& bulkWriter,
				Height height,
				const std::vector<model::TransactionElement>& transactions,
				const MongoTransactionRegistry& registry) {
			std::atomic<size_t> numTotalTransactionDocuments(0);
			auto createDocuments = [height, &registry, &numTotalTransactionDocuments](const auto& transactionElement, auto index) {
				auto metadata = MongoTransactionMetadata(transactionElement, height, index);
				auto documents = mappers::ToDbDocuments(transactionElement.Transaction, metadata, registry);
				numTotalTransactionDocuments += documents.size();
				return documents;
			};
			auto results = bulkWriter.bulkInsert("transactions", transactions, createDocuments).get();
			auto aggregate = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));
			auto numInsertedDocuments = mappers::ToUint32(aggregate.NumInserted);
			if (transactions.size() > numInsertedDocuments || numTotalTransactionDocuments != numInsertedDocuments) {
				CATAPULT_LOG(error)
						<< "only inserted " << numInsertedDocuments << " documents with "
						<< transactions.size() << " transactions and "
						<< numTotalTransactionDocuments << " expected documents at height " << height;
				CATAPULT_THROW_RUNTIME_ERROR_1("could not insert transactions for block at height", height);
			}
		}

		void SaveBlockStatement(
				MongoBulkWriter& bulkWriter,
				Height height,
				const model::BlockStatement& blockStatement,
				const MongoReceiptRegistry& registry) {
			using BulkWriteResultFuture = thread::future<std::vector<thread::future<BulkWriteResult>>>;

			std::vector<BulkWriteResultFuture> futures;
			std::vector<size_t> numExpectedInserts;

			// transaction statements
			numExpectedInserts.emplace_back(blockStatement.TransactionStatements.size());
			futures.emplace_back(bulkWriter.bulkInsert("transactionStatements", blockStatement.TransactionStatements, [height, &registry](
					const auto& pair,
					auto) {
				return mappers::ToDbModel(height, pair.second, registry);
			}));

			// address resolution statements
			numExpectedInserts.emplace_back(blockStatement.AddressResolutionStatements.size());
			futures.emplace_back(bulkWriter.bulkInsert("addressResolutionStatements", blockStatement.AddressResolutionStatements, [height](
					const auto& pair,
					auto) {
				return mappers::ToDbModel(height, pair.second);
			}));

			// mosaic resolution statements
			numExpectedInserts.emplace_back(blockStatement.MosaicResolutionStatements.size());
			futures.emplace_back(bulkWriter.bulkInsert("mosaicResolutionStatements", blockStatement.MosaicResolutionStatements, [height](
					const auto& pair,
					auto) {
				return mappers::ToDbModel(height, pair.second);
			}));

			auto statementsFuture = thread::when_all(std::move(futures)).then([height, numExpectedInserts](auto&& resultsFuture) {
				auto insertResultsContainer = resultsFuture.get();
				auto i = 0u;
				for (auto& insertResults : insertResultsContainer) {
					auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(insertResults.get())));
					auto numInserts = mappers::ToUint32(aggregateResult.NumInserted);
					if (numExpectedInserts[i] != numInserts) {
						CATAPULT_LOG(error)
								<< "only inserted " << numInserts << " documents with "
								<< numExpectedInserts[i] << " expected documents at height " << height;
						CATAPULT_THROW_RUNTIME_ERROR_1("could not insert statements for block at height", height);
					}

					++i;
				}
			});

			statementsFuture.get();
		}

		void DropDocuments(mongocxx::database& database, const std::string& collectionName, const std::string& indexName, Height height) {
			auto blocks = database[collectionName];
			auto filter = document()
					<< indexName
					<< open_document << "$gt" << static_cast<int64_t>(height.unwrap()) << close_document
					<< finalize;
			auto result = blocks.delete_many(filter.view());
			if (result)
				CATAPULT_LOG(info) << "deleted " << result->deleted_count() << " " << collectionName;
			else
				CATAPULT_THROW_RUNTIME_ERROR("delete returned empty result");
		}

		void DropAll(mongocxx::database& database, Height height) {
			DropDocuments(database, "blocks", "block.height", height);
			DropDocuments(database, "transactions", "meta.height", height);
			DropDocuments(database, "transactionStatements", "height", height);
			DropDocuments(database, "addressResolutionStatements", "height", height);
			DropDocuments(database, "mosaicResolutionStatements", "height", height);
		}

		class MongoBlockStorage final : public io::LightBlockStorage {
		public:
			MongoBlockStorage(
					MongoStorageContext& context,
					const MongoTransactionRegistry& transactionRegistry,
					const MongoReceiptRegistry& receiptRegistry)
					: m_context(context)
					, m_transactionRegistry(transactionRegistry)
					, m_receiptRegistry(receiptRegistry)
					, m_database(m_context.createDatabaseConnection())
			{}

		public:
			// region LightBlockStorage

			Height chainHeight() const override {
				auto chainInfoDocument = GetChainInfoDocument(m_database);
				if (mappers::IsEmptyDocument(chainInfoDocument))
					return Height();

				auto heightValue = mappers::GetUint64OrDefault(chainInfoDocument.view(), "height", 0);
				return Height(heightValue);
			}

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

				SaveBlockHeader(m_database, blockElement);
				SaveTransactions(m_context.bulkWriter(), height, blockElement.Transactions, m_transactionRegistry);
				if (blockElement.OptionalStatement)
					SaveBlockStatement(m_context.bulkWriter(), height, *blockElement.OptionalStatement, m_receiptRegistry);

				SetHeight(m_database, blockElement.Block.Height);
			}

			void dropBlocksAfter(Height height) override {
				auto dbHeight = chainHeight();
				if (dbHeight <= height)
					return;

				SetHeight(m_database, height);
				DropAll(m_database, height);
			}

			// endregion

		private:
			MongoStorageContext& m_context;
			const MongoTransactionRegistry& m_transactionRegistry;
			const MongoReceiptRegistry& m_receiptRegistry;
			MongoDatabase m_database;
		};
	}

	std::unique_ptr<io::LightBlockStorage> CreateMongoBlockStorage(
			MongoStorageContext& context,
			const MongoTransactionRegistry& transactionRegistry,
			const MongoReceiptRegistry& receiptRegistry) {
		return std::make_unique<MongoBlockStorage>(context, transactionRegistry, receiptRegistry);
	}
}}
