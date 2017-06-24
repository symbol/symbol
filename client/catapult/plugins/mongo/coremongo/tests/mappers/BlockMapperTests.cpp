#include "src/mappers/BlockMapper.h"
#include "src/mappers/MapperUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace mappers {

	// region ToDbModel

	namespace {
		void AssertCanMapBlock(const model::Block& block, Amount totalFee, int32_t numTransactions) {
			// Arrange:
			auto blockElement = model::BlockElement(block);
			blockElement.EntityHash = test::GenerateRandomData<Hash256_Size>();
			blockElement.GenerationHash = test::GenerateRandomData<Hash256_Size>();

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
					metaView);

			auto blockView = view["block"].get_document().view();
			test::AssertEqualBlockData(block, blockView);
		}
	}

	TEST(BlockMapperTests, CanMapBlockWithoutTransactions) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();

		// Assert:
		AssertCanMapBlock(*pBlock, Amount(0), 0);
	}

	TEST(BlockMapperTests, CanMapBlockWithTransactions) {
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
			return bson_stream::document{}
					<< "height" << ToInt64(difficultyInfo.BlockHeight)
					<< "timestamp" << ToInt64(difficultyInfo.BlockTimestamp)
					<< "difficulty" << ToInt64(difficultyInfo.BlockDifficulty)
					<< bson_stream::finalize;
		}
	}

	TEST(BlockMapperTests, CanMapDifficultyInfo) {
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
