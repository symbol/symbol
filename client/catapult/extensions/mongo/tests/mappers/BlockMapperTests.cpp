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

#include "mongo/src/mappers/BlockMapper.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/FinalizationRound.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoReceiptTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace mappers {

#define TEST_CLASS BlockMapperTests

	// region ToDbModel (block)

	namespace {
		void AssertCanMapBlock(
				const model::Block& block,
				Amount totalFee,
				uint32_t transactionsCount,
				uint32_t totalTransactionsCount,
				uint32_t statementsCount) {
			// Arrange:
			auto blockElement = model::BlockElement(block);
			blockElement.EntityHash = test::GenerateRandomByteArray<Hash256>();
			blockElement.GenerationHash = test::GenerateRandomByteArray<GenerationHash>();
			blockElement.SubCacheMerkleRoots = test::GenerateRandomDataVector<Hash256>(3);

			auto& transactionElements = blockElement.Transactions;
			for (const auto& transaction: block.Transactions()) {
				transactionElements.push_back(model::TransactionElement(transaction));
				transactionElements.back().MerkleComponentHash = test::GenerateRandomByteArray<Hash256>();
			}

			auto transactionMerkleTree = test::CalculateMerkleTree(blockElement.Transactions);
			if (0 < statementsCount)
				blockElement.OptionalStatement = test::GenerateRandomOptionalStatement(statementsCount);

			auto statementMerkleTree = blockElement.OptionalStatement
					? test::CalculateMerkleTreeFromTransactionStatements(*blockElement.OptionalStatement)
					: std::vector<Hash256>();

			// Act:
			auto dbBlock = ToDbModel(blockElement, totalTransactionsCount);

			// Assert:
			auto view = dbBlock.view();
			EXPECT_EQ(2u, test::GetFieldCount(view));

			auto metaView = view["meta"].get_document().view();
			test::AssertEqualBlockMetadata(
					blockElement,
					totalFee,
					{ transactionsCount, totalTransactionsCount, statementsCount },
					transactionMerkleTree,
					statementMerkleTree,
					metaView);

			auto blockView = view["block"].get_document().view();
			test::AssertEqualBlockData(block, blockView);
		}
	}

#define TRAITS_BASED_RECEIPTS_TEST(TEST_NAME) \
	template<size_t Num_Statements> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_WithoutReceipts) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<0>(); } \
	TEST(TEST_CLASS, TEST_NAME##_WithReceipts) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<7>(); } \
	template<size_t Num_Statements> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_RECEIPTS_TEST(CanMapBlockWithoutTransactions) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();

		// Assert:
		AssertCanMapBlock(*pBlock, Amount(0), 0, 0, Num_Statements);

		// Sanity:
		EXPECT_FALSE(model::IsImportanceBlock(pBlock->Type, pBlock->Version));
	}

	TRAITS_BASED_RECEIPTS_TEST(CanMapImportanceBlockWithoutTransactions) {
		// Arrange:
		auto pBlock = test::GenerateImportanceBlockWithTransactions(0);
		auto& blockFooter = model::GetBlockFooter<model::ImportanceBlockFooter>(*pBlock);
		test::FillWithRandomData({ reinterpret_cast<uint8_t*>(&blockFooter), sizeof(model::ImportanceBlockFooter) });

		// Assert:
		AssertCanMapBlock(*pBlock, Amount(0), 0, 0, Num_Statements);

		// Sanity:
		EXPECT_TRUE(model::IsImportanceBlock(pBlock->Type, pBlock->Version));
	}

	TRAITS_BASED_RECEIPTS_TEST(CanMapBlockWithTransactions) {
		// Arrange:
		auto pBlock = test::GenerateBlockWithTransactions(5, Height(123));
		auto totalFee = model::CalculateBlockTransactionsInfo(*pBlock).TotalFee;

		// Assert:
		AssertCanMapBlock(*pBlock, totalFee, 5, 12, Num_Statements);
	}

	// endregion

	// region ToDbModel (finalized block)

	TEST(TEST_CLASS, CanMapFinalizedBlock) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto document = ToDbModel({ FinalizationEpoch(23), FinalizationPoint(97) }, Height(321), hash);
		auto documentView = document.view();

		// Assert:
		EXPECT_EQ(1u, test::GetFieldCount(documentView));

		auto statusView = documentView["block"].get_document().view();
		EXPECT_EQ(4u, test::GetFieldCount(statusView));

		EXPECT_EQ(23u, test::GetUint32(statusView, "finalizationEpoch"));
		EXPECT_EQ(97u, test::GetUint32(statusView, "finalizationPoint"));
		EXPECT_EQ(321u, test::GetUint64(statusView, "height"));
		EXPECT_EQ(hash, test::GetHashValue(statusView, "hash"));
	}

	// endregion
}}}
