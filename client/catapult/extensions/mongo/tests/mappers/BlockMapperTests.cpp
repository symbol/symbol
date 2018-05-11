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

#include "mongo/src/mappers/BlockMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace mappers {

#define TEST_CLASS BlockMapperTests

	// region ToDbModel

	namespace {
		void AssertCanMapBlock(const model::Block& block, Amount totalFee, int32_t numTransactions) {
			// Arrange:
			auto blockElement = model::BlockElement(block);
			blockElement.EntityHash = test::GenerateRandomData<Hash256_Size>();
			blockElement.GenerationHash = test::GenerateRandomData<Hash256_Size>();
			crypto::MerkleHashBuilder builder;
			auto& transactionElements = blockElement.Transactions;
			for (const auto& transaction: block.Transactions()) {
				transactionElements.push_back(model::TransactionElement(transaction));
				transactionElements.back().MerkleComponentHash = test::GenerateRandomData<Hash256_Size>();
				builder.update(transactionElements.back().MerkleComponentHash);
			}

			std::vector<Hash256> merkleTree;
			builder.final(merkleTree);

			// Act:
			auto dbBlock = ToDbModel(blockElement);

			// Assert:
			auto view = dbBlock.view();
			EXPECT_EQ(2u, test::GetFieldCount(view));

			auto metaView = view["meta"].get_document().view();
			test::AssertEqualBlockMetadata(
					blockElement.EntityHash,
					blockElement.GenerationHash,
					totalFee,
					numTransactions,
					merkleTree,
					metaView);

			auto blockView = view["block"].get_document().view();
			test::AssertEqualBlockData(block, blockView);
		}
	}

	TEST(TEST_CLASS, CanMapBlockWithoutTransactions) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();

		// Assert:
		AssertCanMapBlock(*pBlock, Amount(0), 0);
	}

	TEST(TEST_CLASS, CanMapBlockWithTransactions) {
		// Arrange:
		auto pBlock = test::GenerateBlockWithTransactionsAtHeight(5, Height(123));
		Amount totalFee(0);
		for (const auto& transaction : pBlock->Transactions())
			totalFee = totalFee + transaction.Fee;

		// Assert:
		AssertCanMapBlock(*pBlock, totalFee, 5);
	}

	// endregion

	// region ToDifficultyInfo

	namespace {
		state::BlockDifficultyInfo GenerateRandomDifficultyInfo() {
			return state::BlockDifficultyInfo(
					test::GenerateRandomValue<Height>(),
					test::GenerateRandomValue<Timestamp>(),
					test::GenerateRandomValue<Difficulty>());
		}

		auto GenerateValue(const state::BlockDifficultyInfo& difficultyInfo) {
			return bson_stream::document()
					<< "height" << ToInt64(difficultyInfo.BlockHeight)
					<< "timestamp" << ToInt64(difficultyInfo.BlockTimestamp)
					<< "difficulty" << ToInt64(difficultyInfo.BlockDifficulty)
					<< bson_stream::finalize;
		}
	}

	TEST(TEST_CLASS, CanMapDifficultyInfo) {
		// Arrange:
		auto expectedDifficultyInfo = GenerateRandomDifficultyInfo();
		auto value = GenerateValue(expectedDifficultyInfo);

		// Act:
		auto result = ToDifficultyInfo(value.view());

		// Assert:
		EXPECT_EQ(expectedDifficultyInfo, result);
	}

	// endregion
}}}
