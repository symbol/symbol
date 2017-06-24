#include "catapult/model/Elements.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	namespace {
		class MultiBlockInput {
		public:
			MultiBlockInput() {
				m_blocks.push_back(test::GenerateBlockWithTransactionsAtHeight(1, 246));
				m_blocks.push_back(test::GenerateBlockWithTransactionsAtHeight(0, 247));
				m_blocks.push_back(test::GenerateBlockWithTransactionsAtHeight(3, 248));
				m_blocks.push_back(test::GenerateBlockWithTransactionsAtHeight(2, 249));

				for (const auto& pBlock : m_blocks)
					m_elements.push_back(test::BlockToBlockElement(*pBlock));
			}

		public:
			const auto& blocks() const {
				return m_elements;
			}

		private:
			std::vector<std::unique_ptr<Block>> m_blocks;
			std::vector<BlockElement> m_elements;
		};

		struct PredicateParams {
		public:
			PredicateParams(BasicEntityType entityType, Timestamp timestamp, const Hash256& hash)
					: EntityType(entityType)
					, Timestamp(timestamp)
					, Hash(hash)
			{}

		public:
			BasicEntityType EntityType;
			catapult::Timestamp Timestamp;
			Hash256 Hash;
		};

		void AssertEqual(const BlockElement& expected, const WeakEntityInfo& actual, const char* id) {
			// Assert:
			EXPECT_EQ(expected.Block, actual.entity()) << "block at " << id;
			EXPECT_EQ(expected.EntityHash, actual.hash()) << "block at " << id;
		}

		void AssertEqual(const TransactionElement& expected, const WeakEntityInfo& actual, const char* id) {
			// Assert:
			EXPECT_EQ(expected.Transaction, actual.entity()) << "transaction at " << id;
			EXPECT_EQ(expected.EntityHash, actual.hash()) << "transaction at " << id;
		}

		void AssertTransactionsFromBlock(
				const BlockElement& expected,
				size_t numExpected,
				WeakEntityInfos& entityInfos,
				size_t startIndex,
				const char* shortTag) {
			ASSERT_GE(entityInfos.size(), startIndex + numExpected);

			for (auto i = 0u; i < numExpected; ++i) {
				auto tag = std::string(shortTag) + ", " + std::to_string(i);
				AssertEqual(expected.Transactions[i], entityInfos[startIndex + i], tag.c_str());
			}
		}

		void AssertExpectedParameters(const BlockElement& element, const std::vector<PredicateParams>& actualParams, size_t startIndex) {
			auto index = startIndex;
			for (const auto& transactionElement : element.Transactions) {
				EXPECT_EQ(BasicEntityType::Transaction, actualParams[index].EntityType) << "entity type at " << index;
				EXPECT_EQ(transactionElement.Transaction.Deadline, actualParams[index].Timestamp) << "timestamp at " << index;
				EXPECT_EQ(transactionElement.EntityHash, actualParams[index].Hash) << "entity hash at " << index;
				++index;
			}

			EXPECT_EQ(BasicEntityType::Block, actualParams[index].EntityType) << "entity type at " << index;
			EXPECT_EQ(element.Block.Timestamp, actualParams[index].Timestamp) << "timestamp at " << index;
			EXPECT_EQ(element.EntityHash, actualParams[index].Hash) << "entity hash at " << index;
		}
	}

	// region ExtractMatchingEntityInfos

	TEST(ElementsTests, ExtractMatchingEntityInfos_CanExtractAllEntitiesWithoutFilter) {
		// Arrange:
		WeakEntityInfos entityInfos;
		auto input = MultiBlockInput();
		const auto& elements = input.blocks();

		// Act:
		ExtractMatchingEntityInfos(elements, entityInfos, [](auto, auto, const auto&) { return true; });

		// Assert:
		ASSERT_EQ(10u, entityInfos.size());
		AssertTransactionsFromBlock(elements[0], 1, entityInfos, 0, "block 0");
		AssertEqual(elements[0], entityInfos[1], "0");
		AssertEqual(elements[1], entityInfos[2], "1");
		AssertTransactionsFromBlock(elements[2], 3, entityInfos, 3, "block 2");
		AssertEqual(elements[2], entityInfos[6], "2");
		AssertTransactionsFromBlock(elements[3], 2, entityInfos, 7, "block 3");
		AssertEqual(elements[3], entityInfos[9], "3");
	}

	TEST(ElementsTests, ExtractMatchingEntityInfos_CanFilterOutBlocks) {
		// Arrange:
		WeakEntityInfos entityInfos;
		auto input = MultiBlockInput();
		const auto& elements = input.blocks();

		// Act:
		ExtractMatchingEntityInfos(elements, entityInfos, [&elements](auto entityType, auto, const auto& hash) {
			auto isBlock = BasicEntityType::Block == entityType;
			auto isHashMatch = elements[1].EntityHash == hash || elements[3].EntityHash == hash;
			return !isBlock || !isHashMatch;
		});

		// Assert:
		ASSERT_EQ(8u, entityInfos.size());
		AssertTransactionsFromBlock(elements[0], 1, entityInfos, 0, "block 0");
		AssertEqual(elements[0], entityInfos[1], "0");
		AssertTransactionsFromBlock(elements[2], 3, entityInfos, 2, "block 2");
		AssertEqual(elements[2], entityInfos[5], "2");
		AssertTransactionsFromBlock(elements[3], 2, entityInfos, 6, "block 3");
	}

	TEST(ElementsTests, ExtractMatchingEntityInfos_CanFilterOutTransactions) {
		// Arrange:
		WeakEntityInfos entityInfos;
		auto input = MultiBlockInput();
		const auto& elements = input.blocks();

		// Act:
		ExtractMatchingEntityInfos(elements, entityInfos, [&elements](auto entityType, auto, const auto& hash) {
			auto isTransaction = BasicEntityType::Transaction == entityType;
			auto isHashMatch = elements[0].Transactions[0].EntityHash == hash || elements[3].Transactions[1].EntityHash == hash;
			return !isTransaction || !isHashMatch;
		});

		// Assert:
		ASSERT_EQ(8u, entityInfos.size());
		AssertEqual(elements[0], entityInfos[0], "0");
		AssertEqual(elements[1], entityInfos[1], "1");
		AssertTransactionsFromBlock(elements[2], 3, entityInfos, 2, "block 2");
		AssertEqual(elements[2], entityInfos[5], "2");
		AssertEqual(elements[3].Transactions[0], entityInfos[6], "block 3, 0");
		AssertEqual(elements[3], entityInfos[7], "3");
	}

	TEST(ElementsTests, ExtractMatchingEntityInfos_ParamsArePassedToPredicate) {
		// Arrange:
		WeakEntityInfos entityInfos;
		auto input = MultiBlockInput();
		const auto& elements = input.blocks();
		std::vector<PredicateParams> params;

		// Act:
		ExtractMatchingEntityInfos(elements, entityInfos, [&elements, &params](auto entityType, auto timestamp, const auto& hash) {
			params.emplace_back(entityType, timestamp, hash);
			return true;
		});

		// Assert:
		ASSERT_EQ(entityInfos.size(), params.size());
		AssertExpectedParameters(elements[0], params, 0);
		AssertExpectedParameters(elements[1], params, 2);
		AssertExpectedParameters(elements[2], params, 3);
		AssertExpectedParameters(elements[3], params, 7);
	}

	// endregion

	// region ExtractEntityInfos

	TEST(ElementsTests, ExtractEntityInfos_CanExtractAllEntitiesFromBlockWithoutTransactions) {
		// Arrange:
		WeakEntityInfos entityInfos;
		auto pBlock = test::GenerateBlockWithTransactionsAtHeight(0, 246);
		auto element = test::BlockToBlockElement(*pBlock);

		// Act:
		ExtractEntityInfos(element, entityInfos);

		// Assert:
		ASSERT_EQ(1u, entityInfos.size());
		AssertEqual(element, entityInfos[0], "0");
	}

	TEST(ElementsTests, ExtractEntityInfos_CanExtractAllEntitiesFromBlockWithTransactions) {
		// Arrange:
		WeakEntityInfos entityInfos;
		auto pBlock = test::GenerateBlockWithTransactionsAtHeight(3, 246);
		auto element = test::BlockToBlockElement(*pBlock);

		// Act:
		ExtractEntityInfos(element, entityInfos);

		// Assert:
		ASSERT_EQ(4u, entityInfos.size());
		AssertTransactionsFromBlock(element, 3, entityInfos, 0, "block 0");
		AssertEqual(element, entityInfos[3], "0");
	}

	// endregion

	// region ExtractTransactionInfos

	namespace {
		std::shared_ptr<BlockElement> PrepareBlockElement(Block& block) {
			auto i = 0u;
			auto pElement = std::make_shared<BlockElement>(block);
			for (const auto& transaction : block.Transactions()) {
				++i;
				auto txElement = TransactionElement(transaction);
				txElement.EntityHash = { { static_cast<uint8_t>(i * 2) } };
				txElement.MerkleComponentHash = { { static_cast<uint8_t>(i * 3) } };
				pElement->Transactions.push_back(txElement);
			}

			return pElement;
		}

		std::vector<TransactionInfo> ExtractTransactionInfos(const std::shared_ptr<const BlockElement>& pBlockElement) {
			std::vector<TransactionInfo> transactionInfos;
			model::ExtractTransactionInfos(transactionInfos, std::move(pBlockElement));
			return transactionInfos;
		}
	}

	TEST(ElementsTests, CanExtractTransactionInfosFromBlockWithNoTransactions) {
		// Arrange: create a block with no transactions
		auto pBlock = test::GenerateBlockWithTransactionsAtHeight(0, 123);
		auto pElement = PrepareBlockElement(*pBlock);

		// Act:
		auto transactionInfos = ExtractTransactionInfos(std::move(pElement));

		// Sanity:
		EXPECT_FALSE(!!pElement);

		// Assert:
		EXPECT_TRUE(transactionInfos.empty());
	}

	TEST(ElementsTests, CanExtractTransactionInfosFromBlockWithTransactions) {
		// Arrange: create a block with transactions
		constexpr auto Num_Transactions = 5u;
		auto pBlock = test::GenerateBlockWithTransactionsAtHeight(Num_Transactions, 123);
		auto pBlockCopy = test::CopyBlock(*pBlock);
		auto pElement = PrepareBlockElement(*pBlock);

		// Act:
		auto transactionInfos = ExtractTransactionInfos(std::move(pElement));

		// Sanity:
		EXPECT_FALSE(!!pElement);

		// Assert:
		size_t i = 0;
		ASSERT_EQ(Num_Transactions, transactionInfos.size());
		for (const auto& transactionCopy : pBlockCopy->Transactions()) {
			const auto& info = transactionInfos[i];
			EXPECT_EQ(transactionCopy, *info.pEntity) << "transaction at " << i;
			EXPECT_EQ(2 * (i + 1), info.EntityHash[0]) << "entity hash at " << i;
			EXPECT_EQ(3 * (i + 1), info.MerkleComponentHash[0]) << "merkle component hash at " << i;
			++i;
		}

		// Sanity: all transactions were compared
		EXPECT_EQ(Num_Transactions, i);
	}

	TEST(ElementsTests, ExtractTransactionInfosExtendsBlockElementLifetime) {
		// Arrange: create a block with transactions
		constexpr auto Num_Transactions = 5u;
		auto pBlock = test::GenerateBlockWithTransactionsAtHeight(Num_Transactions, 123);
		auto pElement = PrepareBlockElement(*pBlock);

		// Act:
		auto transactionInfos = ExtractTransactionInfos(pElement);

		// Assert: the element reference count was extended for each transaction
		EXPECT_EQ(Num_Transactions + 1, pElement.use_count());

		// Act / Assert: release each transaction and verify the decrease in the element reference count
		for (auto i = 0u; i < Num_Transactions; ++i) {
			transactionInfos[i] = TransactionInfo();
			EXPECT_EQ(Num_Transactions - i, pElement.use_count());
		}

		// Assert: pElement is the only remaining reference
		EXPECT_EQ(1, pElement.use_count());
	}

	TEST(ElementsTests, ExtractTransactionInfosAppendsToDestinationVector) {
		// Arrange: create blocks with transactions
		using Blocks = std::vector<std::unique_ptr<model::Block>>;
		constexpr auto Num_Transactions = 5u;
		constexpr auto Num_Blocks = 3u;
		Blocks blocks;
		Blocks blockCopies;
		for (auto i = 0u; i < Num_Blocks; ++i) {
			blocks.push_back(test::GenerateBlockWithTransactionsAtHeight(Num_Transactions + i, 123 + i));
			blockCopies.push_back(test::CopyBlock(*blocks.back()));
		}

		// Act:
		std::vector<TransactionInfo> transactionInfos;
		for (const auto& pBlock : blocks)
			model::ExtractTransactionInfos(transactionInfos, PrepareBlockElement(*pBlock));

		// Assert:
		size_t i = 0;
		ASSERT_EQ((Num_Transactions + Num_Transactions + Num_Blocks - 1) * Num_Blocks / 2, transactionInfos.size());
		for (const auto& pBlockCopy : blockCopies) {
			size_t j = 0;
			for (const auto& transactionCopy : pBlockCopy->Transactions()) {
				auto message = " at block " + std::to_string(i) + " and transaction " + std::to_string(j);
				const auto& info = transactionInfos[i];
				EXPECT_EQ(transactionCopy, *info.pEntity) << "transaction" << message;
				EXPECT_EQ(2 * (j + 1), info.EntityHash[0]) << "entity hash" << message;
				EXPECT_EQ(3 * (j + 1), info.MerkleComponentHash[0]) << "merkle component hash" << message;
				++j;
				++i;
			}
		}
	}
	// endregion
}}
