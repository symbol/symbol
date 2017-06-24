#include "plugins/mongo/coremongo/src/MongoDbStorage.h"
#include "plugins/mongo/coremongo/src/MongoBulkWriter.h"
#include "plugins/mongo/coremongo/src/MongoTransactionMetadata.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/BlockDifficultyCache.h"
#include "catapult/model/ChainScore.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/test/mongo/MongoTestUtils.h"
#include "tests/test/mongo/mocks/MockTransactionMapper.h"
#include "tests/TestHarness.h"
#include <mongocxx/instance.hpp>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

	// region BlockStorage interface

	namespace {
		constexpr uint64_t Multiple_Blocks_Count = 10;

		std::unique_ptr<MongoDbStorage> CreateMongoDbStorage(std::unique_ptr<MongoTransactionPlugin>&& pTransactionPlugin) {
			auto pWriter = MongoBulkWriter::Create(test::DefaultDbUri(), test::DatabaseName(), test::CreateStartedIoServiceThreadPool(8));
			auto pConfig = std::make_shared<MongoStorageConfiguration>(test::DefaultDbUri(), test::DatabaseName(), pWriter);

			auto pRegistry = std::make_shared<MongoTransactionRegistry>();
			pRegistry->registerPlugin(std::move(pTransactionPlugin));
			return std::make_unique<MongoDbStorage>(pConfig, pRegistry);
		}

		std::unique_ptr<MongoDbStorage> CreateMongoDbStorage() {
			return CreateMongoDbStorage(mocks::CreateMockTransactionMongoPlugin());
		}

		void AssertTransactionElements(
				const std::vector<model::TransactionElement>& expectedElements,
				mongocxx::cursor& transactions,
				Height blockHeight) {
			auto it = transactions.begin();
			auto index = 0u;

			for (const auto& expectedElement : expectedElements) {
				const auto& transaction = reinterpret_cast<const mocks::MockTransaction&>(expectedElement.Transaction);
				const auto& transactionDocument = (*it)["transaction"].get_document().value;
				test::AssertEqualMockTransactionData(transaction, transactionDocument);

				const auto& transactionMeta = (*it)["meta"].get_document().value;
				auto expectedMetadata = MongoTransactionMetadata(
						expectedElement.EntityHash,
						expectedElement.MerkleComponentHash,
						blockHeight,
						index++);
				test::AssertEqualTransactionMetadata(expectedMetadata, transactionMeta);
				++it;
			}
		}

		size_t NumTransactionsByFilter(mongocxx::database& database, const bsoncxx::document::value& filter) {
			auto cursor = database["transactions"].find(filter.view());
			return static_cast<size_t>(std::distance(cursor.begin(), cursor.end()));
		}

		size_t NumTransactionsAtHeight(mongocxx::database& database, Height height) {
			auto filter = document{} << "meta.height" << static_cast<int64_t>(height.unwrap()) << finalize;
			return NumTransactionsByFilter(database, filter);
		}

		size_t NumTransactionsInDatabase() {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];
			return NumTransactionsByFilter(database, document{} << finalize);
		}

		void AssertBlockElement(const model::BlockElement& expectedElement) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];

			const auto& entity = expectedElement.Block;
			Amount totalFee(0);
			for (const auto& transaction : entity.Transactions())
				totalFee = totalFee + transaction.Fee;

			auto filter = document{} << "block.height" << static_cast<int64_t>(entity.Height.unwrap()) << finalize;
			auto result = database["blocks"].find_one(filter.view()).get();
			auto view = result.view();

			// block metadata
			auto metaView = view["meta"].get_document().view();
			test::AssertEqualBlockMetadata(
					expectedElement.EntityHash,
					expectedElement.GenerationHash,
					totalFee,
					static_cast<int32_t>(expectedElement.Transactions.size()),
					metaView);

			// block data
			auto blockView = view["block"].get_document().view();
			test::AssertEqualBlockData(entity, blockView);

			// verify number of transactions
			// note that since the transactions are possibly not saved in the order they appear in the block element,
			// we need to sort them upon retrieval because the assert below expects the same order as in the block element
			EXPECT_EQ(expectedElement.Transactions.size(), NumTransactionsAtHeight(database, entity.Height));
			auto txFilter = document{} << "meta.height" << static_cast<int64_t>(entity.Height.unwrap()) << finalize;
			mongocxx::options::find options;
			options.sort(document{} << "meta.index" << 1 << finalize);
			auto cursor = database["transactions"].find(txFilter.view(), options);
			AssertTransactionElements(expectedElement.Transactions, cursor, expectedElement.Block.Height);
		}

		void AssertNoBlock(mongocxx::database& database, Height height) {
			auto filter = document{} << "block.height" << static_cast<int64_t>(height.unwrap()) << finalize;
			auto result = database["blocks"].find_one(filter.view());
			EXPECT_FALSE(result.is_initialized());
		}

		void AssertNoBlockOrTransactions(Height height) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];

			AssertNoBlock(database, height);
			EXPECT_EQ(0u, NumTransactionsAtHeight(database, height));
		}

		template<typename TClamped>
		TClamped RandomClamped() {
			return TClamped(TClamped::Min().unwrap() + test::Random() % (TClamped::Max() - TClamped::Min()).unwrap());
		}

		std::unique_ptr<model::Block> PrepareDbAndBlock(size_t numTransactions = 7) {
			// Arrange:
			test::PrepareDatabase(test::DatabaseName());
			auto pBlock = test::GenerateBlockWithTransactions(numTransactions);
			auto pRawData = reinterpret_cast<uint8_t*>(pBlock.get());
			test::FillWithRandomData({ pRawData + sizeof(uint32_t), sizeof(model::Block) - sizeof(uint32_t) });
			pBlock->Height = Height(1);
			pBlock->Difficulty = RandomClamped<Difficulty>();
			return pBlock;
		}

		void AssertCanSaveBlock(size_t numTransactions, size_t numDependentDocuments, size_t numExpectedTransactions) {
			// Arrange:
			auto pBlock = PrepareDbAndBlock(numTransactions);
			auto blockElement = test::BlockToBlockElement(*pBlock, test::GenerateRandomData<Hash256_Size>());
			auto pTransactionPlugin = mocks::CreateMockTransactionMongoPlugin(mocks::PluginOptionFlags::Default, numDependentDocuments);
			auto pStorage = CreateMongoDbStorage(std::move(pTransactionPlugin));

			// Act:
			pStorage->saveBlock(blockElement);

			// Assert:
			ASSERT_EQ(Height(1), pStorage->chainHeight());
			AssertBlockElement(blockElement);

			// - check collection sizes
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];
			auto filter = document{} << finalize;
			EXPECT_EQ(1u, database["blocks"].count(filter.view()));
			EXPECT_EQ(numExpectedTransactions, database["transactions"].count(filter.view()));
		}
	}

	TEST(MongoDbStorageTests, CanSaveBlockWithoutTransactions) {
		// Assert:
		AssertCanSaveBlock(0, 0, 0);
	}

	TEST(MongoDbStorageTests, CanSaveBlockWithTransactions) {
		// Assert:
		AssertCanSaveBlock(10, 0, 10);
	}

	TEST(MongoDbStorageTests, CanSaveBlockWithTransactionsContainingDependentDocuments) {
		// Assert:
		AssertCanSaveBlock(10, 3, 40);
	}

	namespace {
		struct PrepareDatabaseMixin {
			PrepareDatabaseMixin() {
				test::PrepareDatabase(test::DatabaseName());
			}
		};

		class TestContext final : public PrepareDatabaseMixin {
		public:
			explicit TestContext(size_t topHeight) : m_pStorage(CreateMongoDbStorage()) {
				for (auto i = 1u; i <= topHeight; ++i) {
					auto transactions = test::GenerateRandomTransactions(10);
					m_blocks.push_back(test::GenerateRandomBlockWithTransactions(test::MakeConst(transactions)));
					m_blocks.back()->Height = Height(i);
					m_blockElements.emplace_back(test::BlockToBlockElement(*m_blocks.back(), test::GenerateRandomData<Hash256_Size>()));
				}
			}

		public:
			io::LightBlockStorage& storage() {
				return *m_pStorage;
			}

			local::api::ChainScoreProvider& chainScoreProvider() {
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
			std::unique_ptr<MongoDbStorage> m_pStorage;
		};
	}

	TEST(MongoDbStorageTests, CanSaveMultipleBlocks) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);

		// Act:
		context.saveBlocks();

		// Assert:
		ASSERT_EQ(Height(Multiple_Blocks_Count), context.storage().chainHeight());
		size_t numExpectedTransactions = 0;
		for (const auto& blockElement : context.elements()) {
			AssertBlockElement(blockElement);
			numExpectedTransactions += blockElement.Transactions.size();
		}

		EXPECT_EQ(numExpectedTransactions, NumTransactionsInDatabase());
	}

	TEST(MongoDbStorageTests, CanDropBlocks) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);
		context.saveBlocks();

		// Act:
		context.storage().dropBlocksAfter(Height(5));

		// Assert:
		ASSERT_EQ(Height(5), context.storage().chainHeight());
		size_t numExpectedTransactions = 0;
		for (const auto& blockElement : context.elements()) {
			if (blockElement.Block.Height <= Height(5)) {
				AssertBlockElement(blockElement);
				numExpectedTransactions += blockElement.Transactions.size();
			} else {
				AssertNoBlockOrTransactions(Height(blockElement.Block.Height));
			}
		}

		EXPECT_EQ(numExpectedTransactions, NumTransactionsInDatabase());
	}

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

	TEST(MongoDbStorageTests, CanLoadHashes) {
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

	TEST(MongoDbStorageTests, LoadHashesRespectsMaxHashes) {
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

	// region ChainScoreProvider interface

	namespace {
		void AssertDbScore(const model::ChainScore& expectedScore) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];

			auto cursor = database["chainInfo"].find({});
			ASSERT_EQ(1u, std::distance(cursor.begin(), cursor.end()));

			auto matchedDocument = database["chainInfo"].find_one({}).get();

			auto scoreLow = static_cast<uint64_t>(matchedDocument.view()["scoreLow"].get_int64().value);
			auto scoreHigh = static_cast<uint64_t>(matchedDocument.view()["scoreHigh"].get_int64().value);
			EXPECT_EQ(expectedScore, model::ChainScore(scoreHigh, scoreLow));
		}
	}

	TEST(MongoDbStorageTests, CanSaveScore) {
		// Arrange:
		TestContext context(0);
		model::ChainScore score(0x12345670u, 0x89abcdefu);

		// Act:
		context.chainScoreProvider().saveScore(score);

		// Assert:
		AssertDbScore(score);
	}

	TEST(MongoDbStorageTests, CanLoadScore) {
		// Arrange:
		TestContext context(0);
		model::ChainScore score(0x12345670u, 0x89abcdefu);
		context.chainScoreProvider().saveScore(score);

		// Act:
		auto result = context.chainScoreProvider().loadScore();

		// Assert:
		EXPECT_EQ(score, result);
	}

	TEST(MongoDbStorageTests, SaveScoreDoesNotOverwriteHeight) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);
		model::ChainScore score(0x12345670u, 0x89abcdefu);

		// Act:
		context.saveBlocks();
		context.chainScoreProvider().saveScore(score);

		// Assert:
		EXPECT_EQ(Height(Multiple_Blocks_Count), context.storage().chainHeight());
	}

	TEST(MongoDbStorageTests, SaveHeightDoesNotOverwriteScore) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);
		model::ChainScore score(0x12345670u, 0x89abcdefu);

		// Act:
		context.chainScoreProvider().saveScore(score);
		context.saveBlocks();

		// Assert:
		AssertDbScore(score);
	}

	// endregion
}}}
