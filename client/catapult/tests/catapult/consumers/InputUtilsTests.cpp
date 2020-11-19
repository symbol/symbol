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

#include "catapult/consumers/InputUtils.h"
#include "tests/catapult/consumers/test/ConsumerTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/TestHarness.h"

using catapult::disruptor::ConsumerInput;

namespace catapult { namespace consumers {

#define TEST_CLASS InputUtilsTests

	namespace {
		auto CreateMultiBlockElements() {
			auto pBlock1 = test::GenerateBlockWithTransactions(1, Height(246));
			auto pBlock2 = test::GenerateBlockWithTransactions(0, Height(247));
			auto pBlock3 = test::GenerateBlockWithTransactions(3, Height(248));
			auto pBlock4 = test::GenerateBlockWithTransactions(2, Height(249));
			return test::CreateBlockElements({ pBlock1.get(), pBlock2.get(), pBlock3.get(), pBlock4.get() });
		}
	}

	// region ExtractTransactionHashes

	namespace {
		const Hash256& GetHash(const BlockElements& elements, size_t elementIndex, size_t txIndex) {
			return elements[elementIndex].Transactions[txIndex].EntityHash;
		}

		bool Contains(const utils::HashPointerSet& hashes, const Hash256& hash) {
			auto hashCopy = hash; // use a copy to ensure pointers are not being compared
			return hashes.cend() != hashes.find(&hashCopy);
		}
	}

	TEST(TEST_CLASS, ExtractTransactionHashes_CanExtractAllTransationHashesFromInput) {
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

	TEST(TEST_CLASS, ExtractBlocks_CanExtractAllBlocksFromInput) {
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

	TEST(TEST_CLASS, ExtractEntityInfos_CanExtractAllEntitiesWhenNoneAreSkipped) {
		// Arrange:
		ConsumerInput input(test::CreateTransactionEntityRange(5));
		const auto& elements = input.transactions();

		// Act:
		model::WeakEntityInfos entityInfos;
		std::vector<size_t> entityInfoElementIndexes;
		ExtractEntityInfos(elements, entityInfos, entityInfoElementIndexes);

		// Assert:
		EXPECT_EQ(std::vector<size_t>({ 0, 1, 2, 3, 4 }), entityInfoElementIndexes);
		ASSERT_EQ(5u, entityInfos.size());

		auto i = 0u;
		for (const auto& entityInfo : entityInfos) {
			AssertEqual(elements[i], entityInfo, std::to_string(i).c_str());
			i++;
		}
	}

	TEST(TEST_CLASS, ExtractEntityInfos_CanExtractZeroEntitiesWhenAllAreSkipped) {
		// Arrange:
		ConsumerInput input(test::CreateTransactionEntityRange(5));
		auto& elements = input.transactions();
		auto i = 0u;
		for (auto& element : elements) {
			element.ResultSeverity = 0 == i % 2 ? disruptor::ConsumerResultSeverity::Failure : disruptor::ConsumerResultSeverity::Neutral;
			++i;
		}

		// Act:
		model::WeakEntityInfos entityInfos;
		std::vector<size_t> entityInfoElementIndexes;
		ExtractEntityInfos(elements, entityInfos, entityInfoElementIndexes);

		// Assert:
		EXPECT_TRUE(entityInfoElementIndexes.empty());
		EXPECT_TRUE(entityInfos.empty());
	}

	TEST(TEST_CLASS, ExtractEntityInfos_CanExtractOnlyNonSkippedElementsWhenSomeAreSkipped) {
		// Arrange:
		ConsumerInput input(test::CreateTransactionEntityRange(5));
		auto& elements = input.transactions();
		elements[0].ResultSeverity = disruptor::ConsumerResultSeverity::Failure;
		elements[2].ResultSeverity = disruptor::ConsumerResultSeverity::Neutral;
		elements[3].ResultSeverity = disruptor::ConsumerResultSeverity::Failure;

		// Act:
		model::WeakEntityInfos entityInfos;
		std::vector<size_t> entityInfoElementIndexes;
		ExtractEntityInfos(elements, entityInfos, entityInfoElementIndexes);

		// Assert:
		EXPECT_EQ(std::vector<size_t>({ 1, 4 }), entityInfoElementIndexes);
		ASSERT_EQ(2u, entityInfos.size());

		AssertEqual(elements[1], entityInfos[0], "0");
		AssertEqual(elements[4], entityInfos[1], "1");
	}

	// endregion

	// region CollectRevertedTransactionInfos

	TEST(TEST_CLASS, CollectRevertedTransactionInfos_ReturnsNoInfosWhenHashesMatchAllTransactions) {
		// Arrange:
		auto transactionInfos = test::CreateTransactionInfos(4);

		// Act:
		auto revertedTransactionsInfos = CollectRevertedTransactionInfos(
				{
					&transactionInfos[0].EntityHash,
					&transactionInfos[1].EntityHash,
					&transactionInfos[2].EntityHash,
					&transactionInfos[3].EntityHash
				},
				test::CopyTransactionInfos(transactionInfos));

		// Assert:
		EXPECT_TRUE(revertedTransactionsInfos.empty());
	}

	TEST(TEST_CLASS, CollectRevertedTransactionInfos_ReturnsAllInfosWhenHashesMatchNoTransactions) {
		// Arrange:
		auto transactionInfos = test::CreateTransactionInfos(4);
		auto hash1 = test::GenerateRandomByteArray<Hash256>();
		auto hash2 = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto revertedTransactionsInfos = CollectRevertedTransactionInfos({ &hash1, &hash2 }, test::CopyTransactionInfos(transactionInfos));

		// Assert:
		ASSERT_EQ(4u, revertedTransactionsInfos.size());
		for (auto i = 0u; i < revertedTransactionsInfos.size(); ++i)
			test::AssertEqual(transactionInfos[i], revertedTransactionsInfos[i], "reverted info " + std::to_string(i));
	}

	TEST(TEST_CLASS, CollectRevertedTransactionInfos_OnlyReturnsInfosWithoutMatchingHashes) {
		// Arrange:
		auto transactionInfos = test::CreateTransactionInfos(4);
		auto hash1 = test::GenerateRandomByteArray<Hash256>();
		auto hash2 = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto revertedTransactionsInfos = CollectRevertedTransactionInfos(
				{ &hash1, &hash2, &transactionInfos[2].EntityHash, &transactionInfos[0].EntityHash },
				test::CopyTransactionInfos(transactionInfos));

		// Assert:
		ASSERT_EQ(2u, revertedTransactionsInfos.size());
		test::AssertEqual(transactionInfos[1], revertedTransactionsInfos[0], "reverted info 0");
		test::AssertEqual(transactionInfos[3], revertedTransactionsInfos[1], "reverted info 1");
	}

	// endregion
}}
