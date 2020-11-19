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

#include "catapult/model/Elements.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS ElementsTests

	// region test utils

	namespace {
		class MultiBlockInput {
		public:
			MultiBlockInput() {
				m_blocks.push_back(test::GenerateBlockWithTransactions(1, Height(246)));
				m_blocks.push_back(test::GenerateBlockWithTransactions(0, Height(247)));
				m_blocks.push_back(test::GenerateBlockWithTransactions(3, Height(248)));
				m_blocks.push_back(test::GenerateBlockWithTransactions(2, Height(249)));

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

		void AssertEqual(const BlockElement& expected, size_t transactionIndex, const WeakEntityInfo& actual, const char* id) {
			// Assert:
			auto message = "transaction at " + std::string(id);
			EXPECT_EQ(expected.Transactions[transactionIndex].Transaction, actual.entity()) << message;
			EXPECT_EQ(expected.Transactions[transactionIndex].EntityHash, actual.hash()) << message;
			EXPECT_EQ(expected.Block, actual.associatedBlockHeader()) << message;
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
				AssertEqual(expected, i, entityInfos[startIndex + i], tag.c_str());
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

	// endregion

	// region ExtractMatchingEntityInfos

	TEST(TEST_CLASS, ExtractMatchingEntityInfos_CanExtractAllEntitiesWithoutFilter) {
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

	TEST(TEST_CLASS, ExtractMatchingEntityInfos_CanFilterOutBlocks) {
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

	TEST(TEST_CLASS, ExtractMatchingEntityInfos_CanFilterOutTransactions) {
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
		AssertEqual(elements[3], 0, entityInfos[6], "block 3, 0");
		AssertEqual(elements[3], entityInfos[7], "3");
	}

	TEST(TEST_CLASS, ExtractMatchingEntityInfos_ParamsArePassedToPredicate) {
		// Arrange:
		WeakEntityInfos entityInfos;
		auto input = MultiBlockInput();
		const auto& elements = input.blocks();
		std::vector<PredicateParams> params;

		// Act:
		ExtractMatchingEntityInfos(elements, entityInfos, [&params](auto entityType, auto timestamp, const auto& hash) {
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

	TEST(TEST_CLASS, ExtractEntityInfos_CanExtractAllEntitiesFromBlockWithoutTransactions) {
		// Arrange:
		WeakEntityInfos entityInfos;
		auto pBlock = test::GenerateBlockWithTransactions(0, Height(246));
		auto element = test::BlockToBlockElement(*pBlock);

		// Act:
		ExtractEntityInfos(element, entityInfos);

		// Assert:
		ASSERT_EQ(1u, entityInfos.size());
		AssertEqual(element, entityInfos[0], "0");
	}

	TEST(TEST_CLASS, ExtractEntityInfos_CanExtractAllEntitiesFromBlockWithTransactions) {
		// Arrange:
		WeakEntityInfos entityInfos;
		auto pBlock = test::GenerateBlockWithTransactions(3, Height(246));
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
				auto transactionElement = TransactionElement(transaction);
				transactionElement.EntityHash = { { static_cast<uint8_t>(i * 2) } };
				transactionElement.MerkleComponentHash = { { static_cast<uint8_t>(i * 3) } };
				auto pAddresses = std::make_shared<UnresolvedAddressSet>();
				pAddresses->emplace(UnresolvedAddress{ { { static_cast<uint8_t>(i * 4) } } });
				transactionElement.OptionalExtractedAddresses = pAddresses;
				pElement->Transactions.push_back(transactionElement);
			}

			return pElement;
		}

		std::vector<TransactionInfo> ExtractTransactionInfos(const std::shared_ptr<const BlockElement>& pBlockElement) {
			std::vector<TransactionInfo> transactionInfos;
			ExtractTransactionInfos(transactionInfos, std::move(pBlockElement));
			return transactionInfos;
		}

		void AssertTransactionElement(const Transaction& expectedTransaction, const TransactionInfo& transactionInfo, size_t id) {
			EXPECT_EQ(expectedTransaction, *transactionInfo.pEntity) << "transaction at " << id;
			EXPECT_EQ(2 * (id + 1), transactionInfo.EntityHash[0]) << "entity hash at " << id;
			EXPECT_EQ(3 * (id + 1), transactionInfo.MerkleComponentHash[0]) << "merkle component hash at " << id;
			ASSERT_EQ(1u, transactionInfo.OptionalExtractedAddresses->size()) << "extracted addresses at " << id;
			ASSERT_EQ(4 * (id + 1), (*transactionInfo.OptionalExtractedAddresses->cbegin())[0]) << "extracted address 0 at " << id;
		}
	}

	TEST(TEST_CLASS, CanExtractTransactionInfosFromBlockWithNoTransactions) {
		// Arrange: create a block with no transactions
		auto pBlock = test::GenerateBlockWithTransactions(0, Height(123));
		auto pElement = PrepareBlockElement(*pBlock);

		// Act:
		auto transactionInfos = ExtractTransactionInfos(std::move(pElement));

		// Sanity:
		EXPECT_FALSE(!!pElement);

		// Assert:
		EXPECT_TRUE(transactionInfos.empty());
	}

	TEST(TEST_CLASS, CanExtractTransactionInfosFromBlockWithTransactions) {
		// Arrange: create a block with transactions
		constexpr auto Num_Transactions = 5u;
		auto pBlock = test::GenerateBlockWithTransactions(Num_Transactions, Height(123));
		auto pBlockCopy = test::CopyEntity(*pBlock);
		auto pElement = PrepareBlockElement(*pBlock);

		// Act:
		auto transactionInfos = ExtractTransactionInfos(std::move(pElement));

		// Sanity:
		EXPECT_FALSE(!!pElement);

		// Assert:
		size_t i = 0;
		ASSERT_EQ(Num_Transactions, transactionInfos.size());
		for (const auto& transactionCopy : pBlockCopy->Transactions()) {
			AssertTransactionElement(transactionCopy, transactionInfos[i], i);
			++i;
		}

		// Sanity: all transactions were compared
		EXPECT_EQ(Num_Transactions, i);
	}

	TEST(TEST_CLASS, ExtractTransactionInfosExtendsBlockElementLifetime) {
		// Arrange: create a block with transactions
		constexpr auto Num_Transactions = 5u;
		auto pBlock = test::GenerateBlockWithTransactions(Num_Transactions, Height(123));
		auto pElement = PrepareBlockElement(*pBlock);

		// Act:
		auto transactionInfos = ExtractTransactionInfos(pElement);

		// Assert: the element reference count was extended for each transaction
		EXPECT_EQ(Num_Transactions + 1, static_cast<size_t>(pElement.use_count()));

		// Act / Assert: release each transaction and verify the decrease in the element reference count
		for (auto i = 0u; i < Num_Transactions; ++i) {
			transactionInfos[i] = TransactionInfo();
			EXPECT_EQ(Num_Transactions - i, static_cast<size_t>(pElement.use_count()));
		}

		// Assert: pElement is the only remaining reference
		EXPECT_EQ(1, pElement.use_count());
	}

	TEST(TEST_CLASS, ExtractTransactionInfosAppendsToDestinationVector) {
		// Arrange: create blocks with transactions
		using Blocks = std::vector<std::unique_ptr<Block>>;
		constexpr auto Num_Transactions = 5u;
		constexpr auto Num_Blocks = 3u;
		Blocks blocks;
		Blocks blockCopies;
		for (auto i = 0u; i < Num_Blocks; ++i) {
			blocks.push_back(test::GenerateBlockWithTransactions(Num_Transactions + i, Height(123 + i)));
			blockCopies.push_back(test::CopyEntity(*blocks.back()));
		}

		// Act:
		std::vector<TransactionInfo> transactionInfos;
		for (const auto& pBlock : blocks)
			ExtractTransactionInfos(transactionInfos, PrepareBlockElement(*pBlock));

		// Assert:
		size_t i = 0;
		ASSERT_EQ((Num_Transactions + Num_Transactions + Num_Blocks - 1) * Num_Blocks / 2, transactionInfos.size());
		for (const auto& pBlockCopy : blockCopies) {
			size_t j = 0;
			for (const auto& transactionCopy : pBlockCopy->Transactions()) {
				AssertTransactionElement(transactionCopy, transactionInfos[i], j);
				++j;
				++i;
			}
		}
	}

	// endregion

	// region MakeTransactionInfo

	TEST(TEST_CLASS, CanMakeTransactionInfoFromTransactionAndTransactionElement) {
		// Arrange:
		auto pTransaction1 = utils::UniqueToShared(test::GenerateRandomTransaction());
		auto transactionElement = TransactionElement(*pTransaction1);
		transactionElement.EntityHash = test::GenerateRandomByteArray<Hash256>();
		transactionElement.MerkleComponentHash = test::GenerateRandomByteArray<Hash256>();
		transactionElement.OptionalExtractedAddresses = std::make_shared<UnresolvedAddressSet>();

		auto pTransaction2 = utils::UniqueToShared(test::GenerateRandomTransaction());

		// Act:
		auto transactionInfo = MakeTransactionInfo(pTransaction2, transactionElement);

		// Assert:
		EXPECT_EQ(pTransaction2.get(), transactionInfo.pEntity.get());
		EXPECT_EQ(transactionElement.EntityHash, transactionInfo.EntityHash);
		EXPECT_EQ(transactionElement.MerkleComponentHash, transactionInfo.MerkleComponentHash);
		EXPECT_EQ(transactionElement.OptionalExtractedAddresses.get(), transactionInfo.OptionalExtractedAddresses.get());
	}

	// endregion
}}
