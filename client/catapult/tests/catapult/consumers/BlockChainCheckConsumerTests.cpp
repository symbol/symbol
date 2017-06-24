#include "catapult/consumers/BlockConsumers.h"
#include "catapult/model/BlockUtils.h"
#include "tests/catapult/consumers/utils/ConsumerTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace consumers {

	namespace {
		constexpr uint32_t Test_Block_Chain_Limit = 20;

		disruptor::ConstBlockConsumer CreateDefaultBlockChainCheckConsumer() {
			return CreateBlockChainCheckConsumer(Test_Block_Chain_Limit);
		}
	}

	TEST(BlockChainCheckConsumerTests, CanProcessZeroEntities) {
		// Assert:
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

	TEST(BlockChainCheckConsumerTests, BlockChainSizeCanBeLessThanBlockLimit) {
		// Assert:
		AssertBlockChainSizeValidation(Test_Block_Chain_Limit - 1, disruptor::CompletionStatus::Normal);
	}

	TEST(BlockChainCheckConsumerTests, BlockChainSizeCanBeEqualToBlockLimit) {
		// Assert:
		AssertBlockChainSizeValidation(Test_Block_Chain_Limit, disruptor::CompletionStatus::Normal);
	}

	TEST(BlockChainCheckConsumerTests, BlockChainSizeCannotBeGreaterThanBlockLimit) {
		// Assert:
		AssertBlockChainSizeValidation(Test_Block_Chain_Limit + 1, disruptor::CompletionStatus::Aborted);
	}

	// endregion

	// region duplicate transactions

	namespace {
		std::unique_ptr<model::Block> CreateBlockFromTransactions(
				const test::ConstTransactions& transactions,
				const std::vector<size_t>& transactionIndexes) {
			test::ConstTransactions copiedTransactions;
			for (auto index : transactionIndexes)
				copiedTransactions.push_back(test::CopyEntity(*transactions[index]));

			return test::GenerateRandomBlockWithTransactions(copiedTransactions);
		}
	}

	TEST(BlockChainCheckConsumerTests, ChainIsInvalidIfOneBlockContainsTheSameTransactionTwice) {
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

	TEST(BlockChainCheckConsumerTests, ChainIsInvalidIfTwoBlocksContainTheSameTransaction) {
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

	TEST(BlockChainCheckConsumerTests, ChainIsValidIfAllTransactionsAreUnique) {
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
		void AssertUnlinkedChain(const std::function<void (model::Block&)>& unlink) {
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

	TEST(BlockChainCheckConsumerTests, AllBlocksInChainMustHaveCorrectHeight) {
		// Assert:
		AssertUnlinkedChain([](auto& block) { block.Height = Height(12 + 4); });
	}

	TEST(BlockChainCheckConsumerTests, AllBlocksInChainMustHaveCorrectPreviousBlockHash) {
		// Assert:
		AssertUnlinkedChain([](auto& block) { ++block.PreviousBlockHash[0]; });
	}

	TEST(BlockChainCheckConsumerTests, AllBlocksInChainMustHaveCorrectHeightInCorrectOrder) {
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
