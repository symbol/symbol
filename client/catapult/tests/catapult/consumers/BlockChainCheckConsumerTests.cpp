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

#include "catapult/consumers/BlockConsumers.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/catapult/consumers/test/ConsumerTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace consumers {

#define TEST_CLASS BlockChainCheckConsumerTests

	namespace {
		constexpr uint32_t Test_Block_Chain_Limit = 20;

		disruptor::ConstBlockConsumer CreateDefaultBlockChainCheckConsumer() {
			return CreateBlockChainCheckConsumer(Test_Block_Chain_Limit, utils::TimeSpan::FromHours(1), []() {
				return Timestamp(100);
			});
		}
	}

	TEST(TEST_CLASS, CanProcessZeroEntities) {
		test::AssertPassthroughForEmptyInput(CreateDefaultBlockChainCheckConsumer());
	}

	// region max chain size

	namespace {
		void AssertBlockChainSizeValidation(uint32_t chainSize, disruptor::CompletionStatus expectedStatus) {
			// Arrange:
			auto elements = test::CreateBlockElements(chainSize);
			test::LinkBlocks(Height(12), elements);
			auto consumer = CreateDefaultBlockChainCheckConsumer();

			// Act:
			auto result = consumer(elements);

			// Assert:
			if (disruptor::CompletionStatus::Normal == expectedStatus)
				test::AssertContinued(result);
			else
				test::AssertAborted(result, Failure_Consumer_Remote_Chain_Too_Many_Blocks);
		}
	}

	TEST(TEST_CLASS, BlockChainSizeCanBeLessThanBlockLimit) {
		AssertBlockChainSizeValidation(Test_Block_Chain_Limit - 1, disruptor::CompletionStatus::Normal);
	}

	TEST(TEST_CLASS, BlockChainSizeCanBeEqualToBlockLimit) {
		AssertBlockChainSizeValidation(Test_Block_Chain_Limit, disruptor::CompletionStatus::Normal);
	}

	TEST(TEST_CLASS, BlockChainSizeCannotBeGreaterThanBlockLimit) {
		AssertBlockChainSizeValidation(Test_Block_Chain_Limit + 1, disruptor::CompletionStatus::Aborted);
	}

	// endregion

	// region future blocks

	namespace {
		void AssertFutureBlocksValidation(
				uint32_t chainSize,
				Timestamp currentTime,
				const utils::TimeSpan& maxBlockFutureTime,
				disruptor::CompletionStatus expectedStatus) {
			// Arrange:
			auto elements = test::CreateBlockElements(chainSize);
			test::LinkBlocks(Height(12), elements);
			auto consumer = CreateBlockChainCheckConsumer(Test_Block_Chain_Limit, maxBlockFutureTime, [currentTime]() {
				return currentTime;
			});

			// Sanity: first block should have timestamp 2 * height but validation is only against *last* block
			EXPECT_EQ(Timestamp(24), (elements.begin())->Block.Timestamp);
			EXPECT_EQ(Timestamp(24 + chainSize - 1), (--elements.end())->Block.Timestamp);

			// Act:
			auto result = consumer(elements);

			// Assert:
			if (disruptor::CompletionStatus::Normal == expectedStatus)
				test::AssertContinued(result);
			else
				test::AssertAborted(result, Failure_Consumer_Remote_Chain_Too_Far_In_Future);
		}
	}

	TEST(TEST_CLASS, ChainTimestampCanBeLessThanAcceptedLimit) {
		// Assert: chain timestamp (28ms) < limit (11ms + 18ms)
		AssertFutureBlocksValidation(5, Timestamp(11), utils::TimeSpan::FromMilliseconds(18), disruptor::CompletionStatus::Normal);
	}

	TEST(TEST_CLASS, ChainTimestampCanBeEqualToAcceptedLimit) {
		// Assert: chain timestamp (28ms) == limit (11ms + 17ms)
		AssertFutureBlocksValidation(5, Timestamp(11), utils::TimeSpan::FromMilliseconds(17), disruptor::CompletionStatus::Normal);
	}

	TEST(TEST_CLASS, ChainTimestampCannotBeGreaterThanAcceptedLimit) {
		// Assert: chain timestamp (28ms) > limit (11ms + 16ms)
		AssertFutureBlocksValidation(5, Timestamp(11), utils::TimeSpan::FromMilliseconds(16), disruptor::CompletionStatus::Aborted);
	}

	// endregion

	// region duplicate transactions

	namespace {
		std::unique_ptr<model::Block> CreateBlockFromTransactions(
				const test::ConstTransactions& transactions,
				const std::vector<size_t>& transactionIndexes) {
			test::ConstTransactions transactionsCopy;
			for (auto index : transactionIndexes)
				transactionsCopy.push_back(test::CopyEntity(*transactions[index]));

			return test::GenerateBlockWithTransactions(transactionsCopy);
		}
	}

	TEST(TEST_CLASS, ChainIsInvalidWhenOneBlockContainsTheSameTransactionTwice) {
		// Arrange: create three blocks where the middle one has duplicate transactions
		auto transactions = test::MakeConst(test::GenerateRandomTransactions(9));
		auto pBlock1 = CreateBlockFromTransactions(transactions, { 0, 1, 2 });
		auto pBlock2 = CreateBlockFromTransactions(transactions, { 3, 4, 3 });
		auto pBlock3 = CreateBlockFromTransactions(transactions, { 6, 7, 8 });
		auto elements = test::CreateBlockElements({ pBlock1.get(), pBlock2.get(), pBlock3.get() });
		auto consumer = CreateDefaultBlockChainCheckConsumer();

		// Act:
		auto result = consumer(elements);

		// Assert:
		test::AssertAborted(result, Failure_Consumer_Remote_Chain_Duplicate_Transactions);
	}

	TEST(TEST_CLASS, ChainIsInvalidWhenTwoBlocksContainTheSameTransaction) {
		// Arrange: create three blocks where the first and third share transactions
		auto transactions = test::MakeConst(test::GenerateRandomTransactions(9));
		auto pBlock1 = CreateBlockFromTransactions(transactions, { 0, 1, 2 });
		auto pBlock2 = CreateBlockFromTransactions(transactions, { 3, 4, 5 });
		auto pBlock3 = CreateBlockFromTransactions(transactions, { 6, 1, 8 });
		auto elements = test::CreateBlockElements({ pBlock1.get(), pBlock2.get(), pBlock3.get() });
		auto consumer = CreateDefaultBlockChainCheckConsumer();

		// Act:
		auto result = consumer(elements);

		// Assert:
		test::AssertAborted(result, Failure_Consumer_Remote_Chain_Duplicate_Transactions);
	}

	TEST(TEST_CLASS, ChainIsValidWhenAllTransactionsAreUnique) {
		// Arrange: create three blocks where all transactions are unique
		auto transactions = test::MakeConst(test::GenerateRandomTransactions(9));
		auto pBlock1 = CreateBlockFromTransactions(transactions, { 0, 1, 2 });
		auto pBlock2 = CreateBlockFromTransactions(transactions, { 3, 4, 5 });
		auto pBlock3 = CreateBlockFromTransactions(transactions, { 6, 7, 8 });
		auto elements = test::CreateBlockElements({ pBlock1.get(), pBlock2.get(), pBlock3.get() });
		auto consumer = CreateDefaultBlockChainCheckConsumer();

		// Act:
		auto result = consumer(elements);

		// Assert:
		test::AssertContinued(result);
	}

	// endregion

	// region chain link

	namespace {
		void AssertUnlinkedChain(const consumer<model::Block&>& unlink) {
			// Arrange: unlink the second and third block
			auto elements = test::CreateBlockElements(4);
			test::LinkBlocks(Height(12), elements);
			unlink(const_cast<model::Block&>(elements[2].Block));
			auto consumer = CreateDefaultBlockChainCheckConsumer();

			// Act:
			auto result = consumer(elements);

			// Assert:
			test::AssertAborted(result, Failure_Consumer_Remote_Chain_Improper_Link);
		}
	}

	TEST(TEST_CLASS, AllBlocksInChainMustHaveCorrectHeight) {
		AssertUnlinkedChain([](auto& block) { block.Height = Height(12 + 4); });
	}

	TEST(TEST_CLASS, AllBlocksInChainMustHaveCorrectPreviousBlockHash) {
		AssertUnlinkedChain([](auto& block) { ++block.PreviousBlockHash[0]; });
	}

	TEST(TEST_CLASS, AllBlocksInChainMustHaveCorrectHeightInCorrectOrder) {
		// Arrange: swap the heights of the second and third block
		auto elements = test::CreateBlockElements(4);
		test::LinkBlocks(Height(12), elements);
		const_cast<model::Block&>(elements[1].Block).Height = Height(12 + 2);
		const_cast<model::Block&>(elements[2].Block).Height = Height(12 + 1);
		auto consumer = CreateDefaultBlockChainCheckConsumer();

		// Act:
		auto result = consumer(elements);

		// Assert:
		test::AssertAborted(result, Failure_Consumer_Remote_Chain_Improper_Link);
	}

	// endregion
}}
