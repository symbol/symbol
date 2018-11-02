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
#include "mongo/src/MongoChainInfoUtils.h"
#include "mongo/src/MongoTransactionMetadata.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/model/EntityHasher.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "mongo/tests/test/mocks/MockTransactionMapper.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

#define TEST_CLASS MongoBlockStorageTests

	namespace {
		constexpr uint64_t Multiple_Blocks_Count = 10;

		std::shared_ptr<io::LightBlockStorage> CreateMongoBlockStorage(std::unique_ptr<MongoTransactionPlugin>&& pTransactionPlugin) {
			return test::CreateStorage<io::LightBlockStorage>(
					std::move(pTransactionPlugin),
					test::DbInitializationType::None,
					mongo::CreateMongoBlockStorage);
		}

		void AssertTransactionElements(
				const std::vector<model::TransactionElement>& expectedElements,
				mongocxx::cursor& transactions,
				Height blockHeight) {
			auto iter = transactions.begin();
			auto index = 0u;

			for (const auto& expectedElement : expectedElements) {
				const auto& transaction = reinterpret_cast<const mocks::MockTransaction&>(expectedElement.Transaction);
				const auto& transactionDocument = (*iter)["transaction"].get_document().value;
				test::AssertEqualMockTransactionData(transaction, transactionDocument);

				const auto& transactionMeta = (*iter)["meta"].get_document().value;
				auto expectedMetadata = MongoTransactionMetadata(expectedElement, blockHeight, index++);
				test::AssertEqualTransactionMetadata(expectedMetadata, transactionMeta);
				++iter;
			}
		}

		size_t NumTransactionsAtHeight(mongocxx::database& database, Height height) {
			auto filter = document() << "meta.height" << static_cast<int64_t>(height.unwrap()) << finalize;
			auto cursor = database["transactions"].find(filter.view());
			return static_cast<size_t>(std::distance(cursor.begin(), cursor.end()));
		}

		void AssertEqual(const model::BlockElement& expectedElement) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];

			const auto& entity = expectedElement.Block;
			Amount totalFee(0);
			for (const auto& transaction : entity.Transactions())
				totalFee = totalFee + transaction.Fee;

			auto merkleTree = model::CalculateMerkleTree(expectedElement.Transactions);

			auto filter = document() << "block.height" << static_cast<int64_t>(entity.Height.unwrap()) << finalize;
			auto result = database["blocks"].find_one(filter.view()).get();
			auto view = result.view();

			// block metadata
			auto metaView = view["meta"].get_document().view();
			auto numTransactions = static_cast<int32_t>(expectedElement.Transactions.size());
			test::AssertEqualBlockMetadata(expectedElement, totalFee, numTransactions, merkleTree, metaView);

			// block data
			auto blockView = view["block"].get_document().view();
			test::AssertEqualBlockData(entity, blockView);

			// verify number of transactions
			// note that since the transactions are possibly not saved in the order they appear in the block element,
			// we need to sort them upon retrieval because the assert below expects the same order as in the block element
			EXPECT_EQ(expectedElement.Transactions.size(), NumTransactionsAtHeight(database, entity.Height));
			auto txFilter = document() << "meta.height" << static_cast<int64_t>(entity.Height.unwrap()) << finalize;
			mongocxx::options::find options;
			options.sort(document() << "meta.index" << 1 << finalize);
			auto cursor = database["transactions"].find(txFilter.view(), options);
			AssertTransactionElements(expectedElement.Transactions, cursor, expectedElement.Block.Height);
		}

		void AssertNoBlock(mongocxx::database& database, Height height) {
			auto filter = document() << "block.height" << static_cast<int64_t>(height.unwrap()) << finalize;
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
			EXPECT_EQ(1u, database["blocks"].count(filter.view()));
			EXPECT_EQ(numExpectedTransactions, static_cast<size_t>(database["transactions"].count(filter.view())));
		}
	}

	TEST(TEST_CLASS, CanSaveBlockWithoutTransactions) {
		// Assert:
		AssertCanSaveBlock(0, 0, 0);
	}

	TEST(TEST_CLASS, CanSaveBlockWithTransactions) {
		// Assert:
		AssertCanSaveBlock(10, 0, 10);
	}

	TEST(TEST_CLASS, CanSaveBlockWithTransactionsContainingDependentDocuments) {
		// Assert:
		AssertCanSaveBlock(10, 3, 40);
	}

	namespace {
		class TestContext final : public test::PrepareDatabaseMixin {
		public:
			explicit TestContext(size_t topHeight) : m_pStorage(CreateMongoBlockStorage(mocks::CreateMockTransactionMongoPlugin())) {
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
	}

	TEST(TEST_CLASS, CanSaveMultipleBlocks) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);

		// Act:
		context.saveBlocks();

		// Assert:
		ASSERT_EQ(Height(Multiple_Blocks_Count), context.storage().chainHeight());
		size_t numExpectedTransactions = 0;
		for (const auto& blockElement : context.elements()) {
			AssertEqual(blockElement);
			numExpectedTransactions += blockElement.Transactions.size();
		}

		test::AssertCollectionSize("transactions", numExpectedTransactions);
	}

	TEST(TEST_CLASS, SaveBlockDoesNotOverwriteScore) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);

		// - set the score
		auto connection = test::CreateDbConnection();
		auto database = connection[test::DatabaseName()];
		auto scoreDocument = document() << "$set"
				<< open_document
					<< "scoreHigh" << static_cast<int64_t>(12)
					<< "scoreLow" << static_cast<int64_t>(98)
				<< close_document << finalize;
		SetChainInfoDocument(database, scoreDocument.view());

		// Act:
		context.saveBlocks();

		// Assert: the score is unchanged
		auto chainInfoDocument = GetChainInfoDocument(database);
		EXPECT_EQ(4u, test::GetFieldCount(chainInfoDocument.view()));
		EXPECT_EQ(12u, test::GetUint64(chainInfoDocument.view(), "scoreHigh"));
		EXPECT_EQ(98u, test::GetUint64(chainInfoDocument.view(), "scoreLow"));
	}

	TEST(TEST_CLASS, CanDropBlocks) {
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
				AssertEqual(blockElement);
				numExpectedTransactions += blockElement.Transactions.size();
			} else {
				AssertNoBlockOrTransactions(Height(blockElement.Block.Height));
			}
		}

		test::AssertCollectionSize("transactions", numExpectedTransactions);
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
}}
