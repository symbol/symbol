/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "MongoChainStatisticUtils.h"
#include "MongoReceiptPlugin.h"
#include "MongoTransactionMetadata.h"
#include "mappers/BlockMapper.h"
#include "mappers/HashMapper.h"
#include "mappers/ResolutionStatementMapper.h"
#include "mappers/TransactionMapper.h"
#include "mappers/TransactionStatementMapper.h"

using namespace bsoncxx::builder::stream;

namespace catapult {
namespace mongo {

    namespace {
        model::HashRange LoadHashes(const mongocxx::database& database, Height height, size_t numHashes)
        {
            auto blocks = database["blocks"];
            auto filter = document() << "block.height" << open_document << "$gte" << static_cast<int64_t>(height.unwrap()) << "$lt"
                                     << static_cast<int64_t>(height.unwrap() + numHashes) << close_document << finalize;

            mongocxx::options::find options;
            options.sort(document() << "block.height" << 1 << finalize);
            options.projection(document() << "meta.hash" << 1 << finalize);

            auto cursor = blocks.find(filter.view(), options);
            return mappers::ToModel(cursor, numHashes);
        }

        void SaveBlockHeader(const mongocxx::database& database, const model::BlockElement& blockElement, uint32_t totalTransactionsCount)
        {
            auto blocks = database["blocks"];
            auto dbBlock = mappers::ToDbModel(blockElement, totalTransactionsCount);

            // in idempotent mode, if blockElement is already in the database, this call will be bypassed
            // so nonzero inserted_count check is proper
            auto result = blocks.insert_one(dbBlock.view()).value().result();
            if (0 == result.inserted_count())
                CATAPULT_THROW_RUNTIME_ERROR("SaveBlockHeader failed: block header was not inserted");
        }

        size_t SaveTransactions(
            MongoBulkWriter& bulkWriter,
            Height height,
            const std::vector<model::TransactionElement>& transactions,
            const MongoTransactionRegistry& registry,
            const MongoErrorPolicy& errorPolicy)
        {
            std::atomic<size_t> totalTransactionsCount(0);
            auto createDocuments = [height, &registry, &totalTransactionsCount](const auto& transactionElement, auto index) {
                auto metadata = MongoTransactionMetadata(transactionElement, height, index);
                auto documents = mappers::ToDbDocuments(transactionElement.Transaction, metadata, registry);
                totalTransactionsCount += documents.size();
                return documents;
            };
            auto results = bulkWriter.bulkInsert("transactions", transactions, createDocuments).get();
            auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(results)));

            auto itemsDescription = "transactions at height " + std::to_string(height.unwrap());
            errorPolicy.checkInserted(totalTransactionsCount, aggregateResult, itemsDescription);
            return totalTransactionsCount;
        }

        void SaveBlockStatement(
            MongoBulkWriter& bulkWriter,
            Height height,
            const model::BlockStatement& blockStatement,
            const MongoReceiptRegistry& registry,
            const MongoErrorPolicy& errorPolicy)
        {
            using BulkWriteResultFuture = thread::future<std::vector<thread::future<BulkWriteResult>>>;

            std::vector<BulkWriteResultFuture> futures;
            std::vector<size_t> numExpectedInserts;

            // transaction statements
            numExpectedInserts.emplace_back(blockStatement.TransactionStatements.size());
            futures.emplace_back(bulkWriter.bulkInsert(
                "transactionStatements",
                blockStatement.TransactionStatements,
                [height, &registry](const auto& pair, auto) { return mappers::ToDbModel(height, pair.second, registry); }));

            // address resolution statements
            numExpectedInserts.emplace_back(blockStatement.AddressResolutionStatements.size());
            futures.emplace_back(bulkWriter.bulkInsert(
                "addressResolutionStatements",
                blockStatement.AddressResolutionStatements,
                [height](const auto& pair, auto) { return mappers::ToDbModel(height, pair.second); }));

            // mosaic resolution statements
            numExpectedInserts.emplace_back(blockStatement.MosaicResolutionStatements.size());
            futures.emplace_back(bulkWriter.bulkInsert(
                "mosaicResolutionStatements",
                blockStatement.MosaicResolutionStatements,
                [height](const auto& pair, auto) { return mappers::ToDbModel(height, pair.second); }));

            auto statementsFuture = thread::when_all(std::move(futures)).then([height, numExpectedInserts, &errorPolicy](auto&& resultsFuture) {
                auto insertResultsContainer = resultsFuture.get();
                auto i = 0u;
                auto itemsDescription = "statements at height " + std::to_string(height.unwrap());
                for (auto& insertResults : insertResultsContainer) {
                    auto aggregateResult = BulkWriteResult::Aggregate(thread::get_all(std::move(insertResults.get())));
                    errorPolicy.checkInserted(numExpectedInserts[i], aggregateResult, itemsDescription);
                    ++i;
                }
            });

            statementsFuture.get();
        }

        class MongoBlockStorage final : public io::LightBlockStorage {
        public:
            MongoBlockStorage(
                MongoStorageContext& context,
                uint32_t maxDropBatchSize,
                const MongoTransactionRegistry& transactionRegistry,
                const MongoReceiptRegistry& receiptRegistry)
                : m_context(context)
                , m_maxDropBatchSize(maxDropBatchSize)
                , m_transactionRegistry(transactionRegistry)
                , m_receiptRegistry(receiptRegistry)
                , m_database(m_context.createDatabaseConnection())
                , m_errorPolicy(m_context.createCollectionErrorPolicy(""))
            {
            }

        public:
            // region LightBlockStorage

            Height chainHeight() const override
            {
                auto chainStatisticDocument = GetChainStatisticDocument(m_database);
                if (mappers::IsEmptyDocument(chainStatisticDocument))
                    return Height();

                auto currentView = chainStatisticDocument.view()["current"].get_document().view();
                auto heightValue = mappers::GetUint64OrDefault(currentView, "height", 0);
                return Height(heightValue);
            }

            model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override
            {
                auto dbHeight = chainHeight();
                if (Height(0) == height || dbHeight < height)
                    return model::HashRange();

                auto numAvailableBlocks = static_cast<size_t>((dbHeight - height).unwrap() + 1);
                auto numHashes = std::min(maxHashes, numAvailableBlocks);

                return LoadHashes(m_database, height, numHashes);
            }

            void saveBlock(const model::BlockElement& blockElement) override
            {
                auto height = blockElement.Block.Height;

                if (MongoErrorPolicy::Mode::Idempotent == m_errorPolicy.mode()) {
                    dropBlocksAfter(height - Height(1));
                    dropAllAfter(height - Height(1)); // forcibly drop orphaned documents
                }

                auto dbHeight = chainHeight();
                if (height != dbHeight + Height(1)) {
                    std::ostringstream out;
                    out << "cannot save block with height " << height << " when storage height is " << dbHeight;
                    CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
                }

                saveBlockInternal(blockElement);
            }

            void dropBlocksAfter(Height height) override
            {
                auto dbHeight = chainHeight();
                if (dbHeight <= height)
                    return;

                setHeight(height);
                dropAll(height, dbHeight);
            }

            // endregion

        private:
            void saveBlockInternal(const model::BlockElement& blockElement)
            {
                auto height = blockElement.Block.Height;
                auto totalTransactionsCount = saveTransactions(height, blockElement.Transactions);

                if (blockElement.OptionalStatement)
                    saveBlockStatement(height, *blockElement.OptionalStatement);

                SaveBlockHeader(m_database, blockElement, static_cast<uint32_t>(totalTransactionsCount));
                setHeight(height);
            }

            size_t saveTransactions(Height height, const std::vector<model::TransactionElement>& transactions)
            {
                return SaveTransactions(m_context.bulkWriter(), height, transactions, m_transactionRegistry, m_errorPolicy);
            }

            void saveBlockStatement(Height height, const model::BlockStatement& blockStatement)
            {
                SaveBlockStatement(m_context.bulkWriter(), height, blockStatement, m_receiptRegistry, m_errorPolicy);
            }

            void setHeight(Height height)
            {
                auto journalHeight = document() << "$set" << open_document << "current.height" << static_cast<int64_t>(height.unwrap())
                                                << close_document << finalize;

                auto result = TrySetChainStatisticDocument(m_database, journalHeight.view());
                m_errorPolicy.checkUpserted(1, result, "height");
            }

            void dropAll(Height newHeight, Height oldHeight)
            {
                auto heightIncrement = Height(m_maxDropBatchSize);

                auto height = oldHeight;
                while (height > heightIncrement && height - heightIncrement > newHeight) {
                    height = height - heightIncrement;
                    dropAllAfter(height);
                }

                if (newHeight != height)
                    dropAllAfter(newHeight);
            }

            void dropAllAfter(Height height)
            {
                CATAPULT_LOG(debug) << "dropping blocks after " << height;

                dropDocuments("blocks", "block.height", height);
                dropDocuments("transactions", "meta.height", height);
                dropDocuments("transactionStatements", "statement.height", height);
                dropDocuments("addressResolutionStatements", "statement.height", height);
                dropDocuments("mosaicResolutionStatements", "statement.height", height);
            }

            void dropDocuments(const std::string& collectionName, const std::string& indexName, Height height)
            {
                auto collection = m_database[collectionName];
                auto filter = document() << indexName << open_document << "$gt" << static_cast<int64_t>(height.unwrap()) << close_document
                                         << finalize;

                mongocxx::options::delete_options options;
                options.write_concern(m_context.bulkWriter().writeOptions());

                auto result = collection.delete_many(filter.view(), options);
                if (result)
                    CATAPULT_LOG(debug) << "deleted " << result->deleted_count() << " " << collectionName;
                else
                    CATAPULT_THROW_RUNTIME_ERROR("delete returned empty result");
            }

        private:
            MongoStorageContext& m_context;
            uint32_t m_maxDropBatchSize;
            const MongoTransactionRegistry& m_transactionRegistry;
            const MongoReceiptRegistry& m_receiptRegistry;
            MongoDatabase m_database;
            MongoErrorPolicy m_errorPolicy;
        };
    }

    std::unique_ptr<io::LightBlockStorage> CreateMongoBlockStorage(
        MongoStorageContext& context,
        uint32_t maxDropBatchSize,
        const MongoTransactionRegistry& transactionRegistry,
        const MongoReceiptRegistry& receiptRegistry)
    {
        return std::make_unique<MongoBlockStorage>(context, maxDropBatchSize, transactionRegistry, receiptRegistry);
    }
}
}
