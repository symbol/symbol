#include "catapult/consumers/InputUtils.h"
#include "tests/catapult/consumers/utils/ConsumerTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

using catapult::disruptor::ConsumerInput;

namespace catapult { namespace consumers {

	namespace {
		auto CreateMultiBlockElements() {
			auto pBlock1 = test::GenerateBlockWithTransactionsAtHeight(1, 246);
			auto pBlock2 = test::GenerateBlockWithTransactionsAtHeight(0, 247);
			auto pBlock3 = test::GenerateBlockWithTransactionsAtHeight(3, 248);
			auto pBlock4 = test::GenerateBlockWithTransactionsAtHeight(2, 249);
			return test::CreateBlockElements({ pBlock1.get(), pBlock2.get(), pBlock3.get(), pBlock4.get() });
		}
	}

	// region ExtractTransactionHashes

	namespace {
		const Hash256& GetHash(const BlockElements& elements, size_t elementIndex, size_t txIndex) {
			return elements[elementIndex].Transactions[txIndex].EntityHash;
		}

		bool Contains(const utils::HashPointerSet& hashes, const Hash256& hash) {
			auto copyHash = hash; // use a copy to ensure pointers are not being compared
			return hashes.cend() != hashes.find(&copyHash);
		}
	}

	TEST(InputUtilsTests, ExtractTransactionHashes_CanExtractAllTransationHashesFromInput) {
		// Arrange:
		auto elements = CreateMultiBlockElements();

		// Act:
		auto hashes = ExtractTransactionHashes(elements);

		// Assert:
		ASSERT_EQ(6u, hashes.size());
		EXPECT_TRUE(Contains(hashes, GetHash(elements, 0, 0)));
		EXPECT_TRUE(Contains(hashes, GetHash(elements, 2, 0)));
		EXPECT_TRUE(Contains(hashes, GetHash(elements, 2, 1)));
		EXPECT_TRUE(Contains(hashes, GetHash(elements, 2, 2)));
		EXPECT_TRUE(Contains(hashes, GetHash(elements, 3, 0)));
		EXPECT_TRUE(Contains(hashes, GetHash(elements, 3, 1)));
	}

	// endregion

	// region ExtractBlocks

	TEST(InputUtilsTests, ExtractBlocks_CanExtractAllBlocksFromInput) {
		// Arrange:
		auto elements = CreateMultiBlockElements();

		// Act:
		auto blocks = ExtractBlocks(elements);

		// Assert:
		EXPECT_EQ(4u, blocks.size());

		for (auto i = 0u; i < blocks.size(); ++i)
			EXPECT_EQ(&elements[i].Block, blocks[i]) << "block at " << i;
	}

	// endregion

	// region ExtractEntityInfos

	namespace {
		void AssertEqual(const model::TransactionElement& expected, const model::WeakEntityInfo& actual, const char* id) {
			// Assert:
			EXPECT_EQ(expected.Transaction, actual.entity()) << "transaction at " << id;
			EXPECT_EQ(expected.EntityHash, actual.hash()) << "transaction at " << id;
		}
	}

	TEST(InputUtilsTests, ExtractEntityInfos_CanExtractAllEntitiesWhenNoneAreSkipped) {
		// Arrange:
		ConsumerInput input(test::CreateTransactionEntityRange(5));
		const auto& elements = input.transactions();

		// Act:
		model::WeakEntityInfos entityInfos;
		ExtractEntityInfos(elements, entityInfos);

		// Assert:
		ASSERT_EQ(5u, entityInfos.size());

		auto i = 0u;
		for (const auto& entityInfo : entityInfos) {
			AssertEqual(elements[i], entityInfo, std::to_string(i).c_str());
			i++;
		}
	}

	TEST(InputUtilsTests, ExtractEntityInfos_CanExtractZeroEntitiesWhenAllAreSkipped) {
		// Arrange:
		ConsumerInput input(test::CreateTransactionEntityRange(5));
		auto& elements = input.transactions();
		for (auto& element : elements)
			element.Skip = true;

		// Act:
		model::WeakEntityInfos entityInfos;
		ExtractEntityInfos(elements, entityInfos);

		// Assert:
		EXPECT_TRUE(entityInfos.empty());
	}

	TEST(InputUtilsTests, ExtractEntityInfos_CanExtractOnlyNonSkippedElementsWhenSomeAreSkipped) {
		// Arrange:
		ConsumerInput input(test::CreateTransactionEntityRange(5));
		auto& elements = input.transactions();
		elements[0].Skip = true;
		elements[2].Skip = true;
		elements[3].Skip = true;

		// Act:
		model::WeakEntityInfos entityInfos;
		ExtractEntityInfos(elements, entityInfos);

		// Assert:
		ASSERT_EQ(2u, entityInfos.size());
		AssertEqual(elements[1], entityInfos[0], "0");
		AssertEqual(elements[4], entityInfos[1], "1");
	}

	// endregion

	// region CollectRevertedTransactionInfos

	namespace {
		model::TransactionInfo GenerateRandomTransactionInfo() {
			return model::TransactionInfo(
					test::GenerateRandomTransaction(),
					test::GenerateRandomData<Hash256_Size>(),
					test::GenerateRandomData<Hash256_Size>());
		}

		TransactionInfos GenerateRandomTransactionInfos(size_t count) {
			TransactionInfos infos;
			for (auto i = 0u; i < count; ++i)
				infos.push_back(GenerateRandomTransactionInfo());

			return infos;
		}

		void AssertAreEqual(const model::TransactionInfo& lhs, const model::TransactionInfo& rhs) {
			// Assert:
			EXPECT_EQ(*lhs.pEntity, *rhs.pEntity);
			EXPECT_EQ(lhs.EntityHash, rhs.EntityHash);
		}
	}

	TEST(InputUtilsTests, CollectRevertedTransactionInfos_ReturnsNoInfosWhenHashesMatchAllTransactions) {
		// Arrange:
		auto infos = GenerateRandomTransactionInfos(4);

		// Act:
		auto revertedTransactionsInfos = CollectRevertedTransactionInfos(
				{ &infos[0].EntityHash, &infos[1].EntityHash, &infos[2].EntityHash, &infos[3].EntityHash },
				test::CopyTransactionInfos(infos));

		// Assert:
		EXPECT_TRUE(revertedTransactionsInfos.empty());
	}

	TEST(InputUtilsTests, CollectRevertedTransactionInfos_ReturnsAllInfosWhenHashesMatchNoTransactions) {
		// Arrange:
		auto infos = GenerateRandomTransactionInfos(4);
		auto hash1 = test::GenerateRandomData<Hash256_Size>();
		auto hash2 = test::GenerateRandomData<Hash256_Size>();

		// Act:
		auto revertedTransactionsInfos = CollectRevertedTransactionInfos(
				{ &hash1, &hash2 },
				test::CopyTransactionInfos(infos));

		// Assert:
		ASSERT_EQ(4u, revertedTransactionsInfos.size());
		for (auto i = 0u; i < revertedTransactionsInfos.size(); ++i)
			AssertAreEqual(infos[i], revertedTransactionsInfos[i]);
	}

	TEST(InputUtilsTests, CollectRevertedTransactionInfos_OnlyReturnsInfosWithoutMatchingHashes) {
		// Arrange:
		auto infos = GenerateRandomTransactionInfos(4);
		auto hash1 = test::GenerateRandomData<Hash256_Size>();
		auto hash2 = test::GenerateRandomData<Hash256_Size>();

		// Act:
		auto revertedTransactionsInfos = CollectRevertedTransactionInfos(
				{ &hash1, &hash2, &infos[2].EntityHash, &infos[0].EntityHash },
				test::CopyTransactionInfos(infos));

		// Assert:
		ASSERT_EQ(2u, revertedTransactionsInfos.size());
		AssertAreEqual(infos[1], revertedTransactionsInfos[0]);
		AssertAreEqual(infos[3], revertedTransactionsInfos[1]);
	}

	// endregion
}}
