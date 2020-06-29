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

#include "mongo/src/MongoBlockStorage.h"
#include "mongo/src/MongoBulkWriter.h"
#include "mongo/src/MongoChainStatisticUtils.h"
#include "mongo/src/MongoReceiptPlugin.h"
#include "mongo/src/MongoTransactionMetadata.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/EntityHasher.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoReceiptTestUtils.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "mongo/tests/test/mocks/MockReceiptMapper.h"
#include "mongo/tests/test/mocks/MockTransactionMapper.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/mocks/MockReceipt.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

#define TEST_CLASS MongoBlockStorageTests

	namespace {
		// region test utils

		constexpr uint64_t Multiple_Blocks_Count = 10;

		struct BlockElementCounts {
			size_t NumTransactions = 0;
			size_t NumTransactionStatements = 0;
			size_t NumAddressResolutionStatements = 0;
			size_t NumMosaicResolutionStatements = 0;

			void AddCounts(const model::BlockElement blockElement) {
				NumTransactions += blockElement.Transactions.size();
				if (blockElement.OptionalStatement) {
					NumTransactionStatements += blockElement.OptionalStatement->TransactionStatements.size();
					NumAddressResolutionStatements += blockElement.OptionalStatement->AddressResolutionStatements.size();
					NumMosaicResolutionStatements += blockElement.OptionalStatement->MosaicResolutionStatements.size();
				}
			}

			size_t NumTotalStatements() const {
				return NumTransactionStatements + NumAddressResolutionStatements + NumMosaicResolutionStatements;
			}
		};

		std::shared_ptr<io::LightBlockStorage> CreateMongoBlockStorage(
				std::unique_ptr<MongoTransactionPlugin>&& pTransactionPlugin,
				MongoErrorPolicy::Mode errorPolicyMode = MongoErrorPolicy::Mode::Strict) {
			auto pMongoReceiptRegistry = std::make_shared<MongoReceiptRegistry>();
			auto mockReceiptType = utils::to_underlying_type(mocks::MockReceipt::Receipt_Type);
			pMongoReceiptRegistry->registerPlugin(mocks::CreateMockReceiptMongoPlugin(mockReceiptType));
			const auto& receiptRegistry = *pMongoReceiptRegistry;
			auto pBlockStorage = test::CreateMongoStorage<io::LightBlockStorage>(
					std::move(pTransactionPlugin),
					test::DbInitializationType::None,
					errorPolicyMode,
					[&receiptRegistry](auto& context, const auto& transactionRegistry) {
						return mongo::CreateMongoBlockStorage(context, transactionRegistry, receiptRegistry);
					});

			return decltype(pBlockStorage)(pBlockStorage.get(), [pMongoReceiptRegistry, pBlockStorage](const auto*) {});
		}

		size_t GetNumEntitiesAtHeight(
				const std::string& collectionName,
				const std::string& indexName,
				Height height,
				mongocxx::database& database) {
			auto filter = document() << indexName << static_cast<int64_t>(height.unwrap()) << finalize;
			auto cursor = database[collectionName].find(filter.view());
			return static_cast<size_t>(std::distance(cursor.begin(), cursor.end()));
		}

		void AssertCollectionSizes(const BlockElementCounts& blockElementCounts) {
			test::AssertCollectionSize("transactions", blockElementCounts.NumTransactions);
			test::AssertCollectionSize("transactionStatements", blockElementCounts.NumTransactionStatements);
			test::AssertCollectionSize("addressResolutionStatements", blockElementCounts.NumAddressResolutionStatements);
			test::AssertCollectionSize("mosaicResolutionStatements", blockElementCounts.NumMosaicResolutionStatements);
		}

		// endregion

		// region transaction

		void AssertTransactionElements(
				const std::vector<model::TransactionElement>& expectedElements,
				mongocxx::cursor& transactions,
				Height blockHeight) {
			auto iter = transactions.begin();
			auto index = 0u;

			for (const auto& expectedElement : expectedElements) {
				const auto& transaction = static_cast<const mocks::MockTransaction&>(expectedElement.Transaction);
				const auto& transactionDocument = (*iter)["transaction"].get_document().value;
				test::AssertEqualMockTransactionData(transaction, transactionDocument);

				const auto& transactionMeta = (*iter)["meta"].get_document().value;
				auto expectedMetadata = MongoTransactionMetadata(expectedElement, blockHeight, index++);
				test::AssertEqualTransactionMetadata(expectedMetadata, transactionMeta);
				++iter;
			}
		}

		// endregion

		// region statements

		void AssertTransactionStatements(
				Height height,
				const std::map<model::ReceiptSource, model::TransactionStatement>& expectedStatements,
				mongocxx::cursor& transactionStatements) {
			auto iter = transactionStatements.begin();
			auto index = 0u;

			for (const auto& pair : expectedStatements) {
				auto statementView = (*iter)["statement"].get_document().view();
				test::AssertEqualTransactionStatement(pair.second, height, statementView, 3, index);
				++iter;
				++index;
			}

			EXPECT_EQ(expectedStatements.size(), index);
		}

		template<typename TTraits>
		void AssertResolutionStatements(
				Height height,
				const std::map<typename TTraits::UnresolvedType, typename TTraits::ResolutionStatementType>& expectedStatements,
				mongocxx::cursor& resolutionStatements) {
			auto iter = resolutionStatements.begin();
			auto index = 0u;

			for (const auto& pair : expectedStatements) {
				auto statementView = (*iter)["statement"].get_document().view();
				TTraits::AssertResolutionStatement(pair.second, height, statementView, 3, index);
				++iter;
				++index;
			}

			EXPECT_EQ(expectedStatements.size(), index);
		}

		void AssertBlockStatement(const model::BlockStatement& blockStatement, Height height, mongocxx::database& database) {
			// note that since the statements are possibly not saved in the order they appear in the block statement,
			// we need to sort them upon retrieval because the assert below expects the same order as in the block statement

			// transaction statements
			auto filter = document() << "statement.height" << static_cast<int64_t>(height.unwrap()) << finalize;
			mongocxx::options::find options1;
			options1.sort(document() << "statement.source.primaryId" << 1 << finalize);
			auto cursor1 = database["transactionStatements"].find(filter.view(), options1);
			AssertTransactionStatements(height, blockStatement.TransactionStatements, cursor1);

			// address resolution statements
			mongocxx::options::find options2;
			options2.sort(document() << "statement.unresolved" << 1 << finalize);
			auto cursor2 = database["addressResolutionStatements"].find(filter.view(), options2);
			AssertResolutionStatements<test::AddressResolutionTraits>(height, blockStatement.AddressResolutionStatements, cursor2);

			// mosaic resolution statements
			auto cursor3 = database["mosaicResolutionStatements"].find(filter.view(), options2);
			AssertResolutionStatements<test::MosaicResolutionTraits>(height, blockStatement.MosaicResolutionStatements, cursor3);
		}

		void AssertNoBlockStatement(Height height, mongocxx::database& database) {
			EXPECT_EQ(0u, GetNumEntitiesAtHeight("transactionStatements", "height", height, database));
			EXPECT_EQ(0u, GetNumEntitiesAtHeight("addressResolutionStatements", "height", height, database));
			EXPECT_EQ(0u, GetNumEntitiesAtHeight("mosaicResolutionStatements", "height", height, database));
		}

		// endregion

		// region block element

		void AssertEqual(const model::BlockElement& expectedElement) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];

			const auto& block = expectedElement.Block;
			auto totalFee = model::CalculateBlockTransactionsInfo(block).TotalFee;

			auto filter = document() << "block.height" << static_cast<int64_t>(block.Height.unwrap()) << finalize;
			auto result = database["blocks"].find_one(filter.view()).value();
			auto view = result.view();

			// block metadata
			auto numTransactions = static_cast<int32_t>(expectedElement.Transactions.size());
			auto transactionMerkleTree = model::CalculateMerkleTree(expectedElement.Transactions);
			int32_t numStatements = 0;
			std::vector<Hash256> statementMerkleTree;
			if (expectedElement.OptionalStatement) {
				const auto& blockStatement = *expectedElement.OptionalStatement;
				numStatements = static_cast<int32_t>(model::CountTotalStatements(blockStatement));
				statementMerkleTree = model::CalculateMerkleTree(blockStatement);
			}

			auto metaView = view["meta"].get_document().view();
			test::AssertEqualBlockMetadata(
					expectedElement,
					totalFee,
					numTransactions,
					numStatements,
					transactionMerkleTree,
					statementMerkleTree,
					metaView);

			// block data
			auto blockView = view["block"].get_document().view();
			test::AssertEqualBlockData(block, blockView);

			// verify number of transactions
			// note that since the transactions are possibly not saved in the order they appear in the block element,
			// we need to sort them upon retrieval because the assert below expects the same order as in the block element
			EXPECT_EQ(expectedElement.Transactions.size(), GetNumEntitiesAtHeight("transactions", "meta.height", block.Height, database));
			auto txFilter = document() << "meta.height" << static_cast<int64_t>(block.Height.unwrap()) << finalize;
			mongocxx::options::find options;
			options.sort(document() << "meta.index" << 1 << finalize);
			auto cursor = database["transactions"].find(txFilter.view(), options);
			AssertTransactionElements(expectedElement.Transactions, cursor, expectedElement.Block.Height);

			// block statement
			if (expectedElement.OptionalStatement)
				AssertBlockStatement(*expectedElement.OptionalStatement, block.Height, database);
			else
				AssertNoBlockStatement(block.Height, database);
		}

		void AssertNoBlock(mongocxx::database& database, Height height) {
			auto filter = document() << "block.height" << static_cast<int64_t>(height.unwrap()) << finalize;
			auto result = database["blocks"].find_one(filter.view());
			EXPECT_FALSE(result.has_value());
		}

		void AssertNoBlockOrTransactions(Height height) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];

			AssertNoBlock(database, height);
			EXPECT_EQ(0u, GetNumEntitiesAtHeight("transactions", "meta.height", height, database));
		}

		// endregion

		// region PrepareDbAndBlock

		template<typename TClamped>
		TClamped RandomClamped() {
			return TClamped(TClamped::Min().unwrap() + test::Random() % (TClamped::Max() - TClamped::Min()).unwrap());
		}

		model::TransactionStatement CreateTransactionStatement(const model::ReceiptSource& source) {
			model::TransactionStatement statement(source);
			mocks::MockReceipt receipt{};
			receipt.Size = sizeof(mocks::MockReceipt);
			receipt.Type = mocks::MockReceipt::Receipt_Type;
			receipt.Payload[0] = static_cast<uint8_t>(source.SecondaryId + 1);
			statement.addReceipt(receipt);
			return statement;
		}

		model::AddressResolutionStatement CreateAddressResolutionStatement(const UnresolvedAddress& address, uint32_t index) {
			model::AddressResolutionStatement statement(address);
			statement.addResolution(test::GenerateRandomByteArray<Address>(), model::ReceiptSource(index, index + 1));
			return statement;
		}

		model::MosaicResolutionStatement CreateMosaicResolutionStatement(UnresolvedMosaicId mosaicId, uint32_t index) {
			model::MosaicResolutionStatement statement(mosaicId);
			statement.addResolution(test::GenerateRandomValue<MosaicId>(), model::ReceiptSource(index, index + 1));
			return statement;
		}

		void AddStatements(model::BlockElement& blockElement, const BlockElementCounts& blockElementCounts) {
			if (0 == blockElementCounts.NumTotalStatements())
				return;

			auto pBlockStatement = std::make_shared<model::BlockStatement>();
			for (auto i = 0u; i < blockElementCounts.NumTransactionStatements; ++i) {
				model::ReceiptSource source(i, i + 1);
				pBlockStatement->TransactionStatements.emplace(source, CreateTransactionStatement(source));
			}

			for (uint8_t i = 0u; i < blockElementCounts.NumAddressResolutionStatements; ++i) {
				UnresolvedAddress address{ { { i } } };
				pBlockStatement->AddressResolutionStatements.emplace(address, CreateAddressResolutionStatement(address, i));
			}

			for (uint8_t i = 0u; i < blockElementCounts.NumMosaicResolutionStatements; ++i) {
				UnresolvedMosaicId mosaicId(i);
				pBlockStatement->MosaicResolutionStatements.emplace(mosaicId, CreateMosaicResolutionStatement(mosaicId, i));
			}

			blockElement.OptionalStatement = pBlockStatement;
		}

		std::unique_ptr<model::Block> PrepareDbAndBlock(const BlockElementCounts& blockElementCounts) {
			// Arrange:
			test::PrepareDatabase(test::DatabaseName());
			auto pBlock = test::GenerateBlockWithTransactions(blockElementCounts.NumTransactions);
			auto pRawData = reinterpret_cast<uint8_t*>(pBlock.get());
			test::FillWithRandomData({ pRawData + sizeof(uint32_t), sizeof(model::BlockHeader) - sizeof(uint32_t) });
			pBlock->Height = Height(1);
			pBlock->Difficulty = RandomClamped<Difficulty>();
			return pBlock;
		}

		// endregion

		// region TestContext

		class TestContext final : public test::PrepareDatabaseMixin {
		public:
			explicit TestContext(size_t topHeight, MongoErrorPolicy::Mode errorPolicyMode = MongoErrorPolicy::Mode::Strict)
					: m_pStorage(CreateMongoBlockStorage(mocks::CreateMockTransactionMongoPlugin(), errorPolicyMode)) {
				for (auto i = 1u; i <= topHeight; ++i) {
					auto transactions = test::GenerateRandomTransactions(10);
					m_blocks.push_back(test::GenerateBlockWithTransactions(transactions));
					m_blocks.back()->Height = Height(i);
					auto blockElement = test::BlockToBlockElement(*m_blocks.back(), test::GenerateRandomByteArray<Hash256>());
					AddStatements(blockElement, { 0, 1, 2, 3});
					m_blockElements.emplace_back(blockElement);
				}
			}

		public:
			io::LightBlockStorage& storage() {
				return *m_pStorage;
			}

			void saveBlocks() {
				for (const auto& blockElement : m_blockElements)
					storage().saveBlock(blockElement);
			}

			const std::vector<model::BlockElement>& elements() {
				return m_blockElements;
			}

		private:
			std::vector<std::unique_ptr<model::Block>> m_blocks;
			std::vector<model::BlockElement> m_blockElements;
			std::shared_ptr<io::LightBlockStorage> m_pStorage;
		};

		// endregion
	}

	// DEFINE_BLOCK_STORAGE_TESTS cannot be used because it assumes BlockStorage but MongoBlockStorage only implements LightBlockStorage

	// region chainHeight

	TEST(TEST_CLASS, ChainHeightIsInitiallyUnset) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);

		// Act:
		auto chainHeight = context.storage().chainHeight();

		// Assert:
		EXPECT_EQ(Height(), chainHeight);
	}

	TEST(TEST_CLASS, ChainHeightIsSetToBlockCountOnSave) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);
		context.saveBlocks();

		// Act:
		auto chainHeight = context.storage().chainHeight();

		// Assert:
		EXPECT_EQ(Height(Multiple_Blocks_Count), chainHeight);
	}

	// endregion

	// region loadHashesFrom

	namespace {
		template<typename TIter>
		void AssertHashes(TIter startIter, TIter endIter, const model::HashRange& hashes) {
			auto hashIter = hashes.begin();
			for (; startIter != endIter; ++startIter) {
				const auto& blockElement = *startIter;
				EXPECT_EQ(blockElement.EntityHash, *hashIter);
				++hashIter;
			}
		}
	}

	TEST(TEST_CLASS, CanLoadHashes) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);
		context.saveBlocks();

		// Act:
		auto hashes = context.storage().loadHashesFrom(Height(5), 100);

		// Assert:
		ASSERT_EQ(Height(Multiple_Blocks_Count), context.storage().chainHeight());
		EXPECT_EQ(Multiple_Blocks_Count - 4, hashes.size());
		auto start = context.elements().begin();
		std::advance(start, 4);
		AssertHashes(start, context.elements().end(), hashes);
	}

	TEST(TEST_CLASS, LoadHashesRespectsMaxHashes) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);
		context.saveBlocks();

		// Act:
		auto hashes = context.storage().loadHashesFrom(Height(5), 3);

		// Assert:
		ASSERT_EQ(Height(Multiple_Blocks_Count), context.storage().chainHeight());
		EXPECT_EQ(3u, hashes.size());
		auto start = context.elements().begin();
		std::advance(start, 4);
		auto end = start;
		std::advance(end, 3);
		AssertHashes(start, end, hashes);
	}

	// endregion

	// region saveBlock

	namespace {
		void AssertCanSaveBlock(
				const BlockElementCounts& blockElementCounts,
				size_t numDependentDocuments,
				size_t numExpectedTransactions) {
			// Arrange:
			auto pBlock = PrepareDbAndBlock(blockElementCounts);
			auto blockElement = test::BlockToBlockElement(*pBlock, test::GenerateRandomByteArray<Hash256>());
			AddStatements(blockElement, blockElementCounts);
			auto pTransactionPlugin = mocks::CreateMockTransactionMongoPlugin(mocks::PluginOptionFlags::Default, numDependentDocuments);
			auto pStorage = CreateMongoBlockStorage(std::move(pTransactionPlugin));

			// Act:
			pStorage->saveBlock(blockElement);

			// Assert:
			ASSERT_EQ(Height(1), pStorage->chainHeight());
			AssertEqual(blockElement);

			// - check collection sizes
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];
			auto filter = document() << finalize;
			EXPECT_EQ(1u, database["blocks"].count_documents(filter.view()));
			EXPECT_EQ(numExpectedTransactions, static_cast<size_t>(database["transactions"].count_documents(filter.view())));
		}
	}

	TEST(TEST_CLASS, CanSaveBlockWithoutTransactions) {
		AssertCanSaveBlock({ 0, 0, 0, 0 }, 0, 0);
	}

	TEST(TEST_CLASS, CanSaveBlockWithTransactionsButNoStatements) {
		AssertCanSaveBlock({ 10, 0, 0, 0 }, 0, 10);
	}

	TEST(TEST_CLASS, CanSaveBlockWithTransactionsContainingDependentDocuments) {
		AssertCanSaveBlock({ 10, 0, 0, 0 }, 3, 40);
	}

	// note that all statements are created prior to saveBlock, so it does not add anything to test with dependent documents

	TEST(TEST_CLASS, CanSaveBlockWithTransactionsAndOnlyTransactionStatements) {
		AssertCanSaveBlock({ 10, 3, 0, 0 }, 0, 10);
	}

	TEST(TEST_CLASS, CanSaveBlockWithTransactionsAndOnlyAddressResolutionStatements) {
		AssertCanSaveBlock({ 10, 0, 3, 0 }, 0, 10);
	}

	TEST(TEST_CLASS, CanSaveBlockWithTransactionsAndOnlyMosaicResolutionStatements) {
		AssertCanSaveBlock({ 10, 0, 0, 3 }, 0, 10);
	}

	TEST(TEST_CLASS, CanSaveBlockWithTransactionsAndAllStatements) {
		AssertCanSaveBlock({ 10, 4, 2, 3 }, 0, 10);
	}

	TEST(TEST_CLASS, CanSaveMultipleBlocks) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);

		// Act:
		context.saveBlocks();

		// Assert:
		ASSERT_EQ(Height(Multiple_Blocks_Count), context.storage().chainHeight());
		BlockElementCounts blockElementCounts;
		for (const auto& blockElement : context.elements()) {
			AssertEqual(blockElement);
			blockElementCounts.AddCounts(blockElement);
		}

		AssertCollectionSizes(blockElementCounts);
	}

	TEST(TEST_CLASS, SaveBlockDoesNotOverwriteScore) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);

		// - set the score
		auto connection = test::CreateDbConnection();
		auto database = connection[test::DatabaseName()];
		auto scoreDocument = document()
				<< "$set" << open_document
					<< "current.scoreHigh" << static_cast<int64_t>(12)
					<< "current.scoreLow" << static_cast<int64_t>(98)
				<< close_document << finalize;
		TrySetChainStatisticDocument(database, scoreDocument.view());

		// Act:
		context.saveBlocks();

		// Assert: the score is unchanged
		auto chainStatisticDocument = GetChainStatisticDocument(database);
		auto currentView = chainStatisticDocument.view()["current"].get_document().view();
		EXPECT_EQ(3u, test::GetFieldCount(currentView));
		EXPECT_EQ(12u, test::GetUint64(currentView, "scoreHigh"));
		EXPECT_EQ(98u, test::GetUint64(currentView, "scoreLow"));
	}

	TEST(TEST_CLASS, CannotSaveSameBlockTwiceWhenErrorModeIsStrict) {
		// Arrange:
		auto pTransactionPlugin = mocks::CreateMockTransactionMongoPlugin(mocks::PluginOptionFlags::Default, 0);
		auto pStorage = CreateMongoBlockStorage(std::move(pTransactionPlugin));

		auto pBlock = PrepareDbAndBlock({ 3, 0, 0, 0 });
		auto blockElement = test::BlockToBlockElement(*pBlock, test::GenerateRandomByteArray<Hash256>());
		pStorage->saveBlock(blockElement);

		// Act + Assert:
		EXPECT_THROW(pStorage->saveBlock(blockElement), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanSaveSameBlockTwiceWhenErrorModeIsIdempotent) {
		// Arrange:
		auto pTransactionPlugin = mocks::CreateMockTransactionMongoPlugin(mocks::PluginOptionFlags::Default, 0);
		auto pStorage = CreateMongoBlockStorage(std::move(pTransactionPlugin), MongoErrorPolicy::Mode::Idempotent);

		auto pBlock = PrepareDbAndBlock({ 3, 0, 0, 0 });
		auto blockElement = test::BlockToBlockElement(*pBlock, test::GenerateRandomByteArray<Hash256>());
		pStorage->saveBlock(blockElement);

		// Act:
		pStorage->saveBlock(blockElement);

		// Assert:
		ASSERT_EQ(Height(1), pStorage->chainHeight());

		AssertEqual(blockElement);

		// - check collection sizes
		auto connection = test::CreateDbConnection();
		auto database = connection[test::DatabaseName()];
		auto filter = document() << finalize;
		EXPECT_EQ(1u, database["blocks"].count_documents(filter.view()));
		EXPECT_EQ(3u, static_cast<size_t>(database["transactions"].count_documents(filter.view())));
	}

	TEST(TEST_CLASS, CanRewriteBlockAtLastBlockHeightWhenErrorModeIsIdempotent) {
		// Arrange: create new block with same height as db
		auto transactions = test::GenerateRandomTransactions(10);
		auto pBlock = test::GenerateBlockWithTransactions(transactions);
		pBlock->Height = Height(Multiple_Blocks_Count);
		auto newBlockElement = test::BlockToBlockElement(*pBlock, test::GenerateRandomByteArray<Hash256>());
		AddStatements(newBlockElement, { 0, 1, 2, 3 });

		// - prepare db with blocks
		TestContext context(Multiple_Blocks_Count, MongoErrorPolicy::Mode::Idempotent);
		context.saveBlocks();

		// Sanity:
		ASSERT_EQ(Height(Multiple_Blocks_Count), context.storage().chainHeight());

		// Act:
		context.storage().saveBlock(newBlockElement);

		// Assert: unmodified block elements
		ASSERT_EQ(Height(Multiple_Blocks_Count), context.storage().chainHeight());

		BlockElementCounts blockElementCounts;
		for (auto i = 0u; i < context.elements().size() - 1; ++i) {
			const auto& blockElement = context.elements()[i];
			AssertEqual(blockElement);
			blockElementCounts.AddCounts(blockElement);
		}

		// - new block element
		AssertEqual(newBlockElement);
		blockElementCounts.AddCounts(newBlockElement);

		AssertCollectionSizes(blockElementCounts);
	}

	namespace {
		void AssertCannotSaveOutOfOrderBlock(MongoErrorPolicy::Mode errorPolicyMode) {
			// Arrange:
			auto pTransactionPlugin = mocks::CreateMockTransactionMongoPlugin(mocks::PluginOptionFlags::Default, 0);
			auto pStorage = CreateMongoBlockStorage(std::move(pTransactionPlugin), errorPolicyMode);

			// - prepare and save a block with height 1
			auto pBlock1 = PrepareDbAndBlock({ 3, 0, 0, 0 });
			auto blockElement1 = test::BlockToBlockElement(*pBlock1, test::GenerateRandomByteArray<Hash256>());
			pStorage->saveBlock(blockElement1);

			// - prepare a block with height 3
			auto pBlock2 = PrepareDbAndBlock({ 3, 0, 0, 0 });
			pBlock2->Height = Height(3);
			auto blockElement2 = test::BlockToBlockElement(*pBlock2, test::GenerateRandomByteArray<Hash256>());

			// Act + Assert:
			EXPECT_THROW(pStorage->saveBlock(blockElement2), catapult_invalid_argument);
		}
	}

	TEST(TEST_CLASS, CannotSaveOutOfOrderBlockWhenModeIsStrict) {
		AssertCannotSaveOutOfOrderBlock(MongoErrorPolicy::Mode::Strict);
	}

	TEST(TEST_CLASS, CannotSaveOutOfOrderBlockWhenModeIsIdempotent) {
		AssertCannotSaveOutOfOrderBlock(MongoErrorPolicy::Mode::Idempotent);
	}

	// endregion

	// region dropBlocksAfter

	TEST(TEST_CLASS, CanDropBlocks) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);
		context.saveBlocks();

		// Act:
		context.storage().dropBlocksAfter(Height(5));

		// Assert:
		ASSERT_EQ(Height(5), context.storage().chainHeight());
		BlockElementCounts blockElementCounts;
		for (const auto& blockElement : context.elements()) {
			if (blockElement.Block.Height <= Height(5)) {
				AssertEqual(blockElement);
				blockElementCounts.AddCounts(blockElement);
			} else {
				AssertNoBlockOrTransactions(blockElement.Block.Height);
			}
		}

		AssertCollectionSizes(blockElementCounts);
	}

	// endregion
}}
