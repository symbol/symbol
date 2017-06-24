#include "ConsumerTestUtils.h"
#include "ConsumerInputFactory.h"
#include "sdk/src/extensions/BlockExtensions.h"
#include "catapult/model/EntityHasher.h"
#include "tests/test/other/DisruptorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region CreateBlockElements

	BlockElementsInputFacade CreateBlockElements(size_t numBlocks) {
		return BlockElementsInputFacade(CreateConsumerInputWithBlocks(numBlocks, disruptor::InputSource::Unknown));
	}

	BlockElementsInputFacade CreateBlockElements(const std::vector<const model::Block*>& blocks) {
		return BlockElementsInputFacade(CreateConsumerInputFromBlocks(blocks));
	}

	// endregion

	// region CreateTransactionElements

	TransactionElementsInputFacade CreateTransactionElements(size_t numTransactions) {
		return TransactionElementsInputFacade(CreateConsumerInputWithTransactions(numTransactions, disruptor::InputSource::Unknown));
	}

	TransactionElementsInputFacade CreateTransactionElements(const std::vector<const model::Transaction*>& transactions) {
		return TransactionElementsInputFacade(CreateConsumerInputFromTransactions(transactions));
	}

	// endregion

	// region LinkBlocks

	void LinkBlocks(const model::Block& parentBlock, model::Block& childBlock) {
		childBlock.Height = parentBlock.Height + Height(1);
		childBlock.Timestamp = parentBlock.Timestamp + Timestamp(1);
		childBlock.PreviousBlockHash = model::CalculateHash(parentBlock);
	}

	void LinkBlocks(Height chainHeight, disruptor::BlockElements& blockElements) {
		auto* pParentBlock = const_cast<model::Block*>(&blockElements[0].Block);
		extensions::UpdateBlockTransactionsHash(*pParentBlock);
		pParentBlock->Height = chainHeight;

		for (auto i = 1u; i < blockElements.size(); ++i) {
			auto& block = const_cast<model::Block&>(blockElements[i].Block);
			extensions::UpdateBlockTransactionsHash(block);
			LinkBlocks(*pParentBlock, block);
			pParentBlock = &block;
		}
	}

	// endregion

	// region ConsumerResult Assertions

	void AssertConsumed(const disruptor::ConsumerResult& result) {
		EXPECT_EQ(disruptor::CompletionStatus::Consumed, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
	}

	void AssertAborted(const disruptor::ConsumerResult& result, validators::ValidationResult validationResult) {
		EXPECT_EQ(disruptor::CompletionStatus::Aborted, result.CompletionStatus);
		EXPECT_EQ(validationResult, static_cast<validators::ValidationResult>(result.CompletionCode));
	}

	// endregion

	// region AssertPassthroughForEmptyInput

	namespace {
		template<typename TConsumer, typename TInput>
		void AssertPassthroughForEmptyInput(const TConsumer& consumer, TInput&& input) {
			// Sanity:
			EXPECT_TRUE(input.empty());

			// Act:
			auto result = consumer(input);

			// Assert:
			test::AssertAborted(result, consumers::Failure_Consumer_Empty_Input);
			EXPECT_TRUE(input.empty());
		}
	}

	void AssertPassthroughForEmptyInput(const disruptor::BlockConsumer& consumer) {
		// Assert:
		AssertPassthroughForEmptyInput(consumer, disruptor::BlockElements());
	}

	void AssertPassthroughForEmptyInput(const disruptor::TransactionConsumer& consumer) {
		// Assert:
		AssertPassthroughForEmptyInput(consumer, disruptor::TransactionElements());
	}

	void AssertPassthroughForEmptyInput(const disruptor::DisruptorConsumer& consumer) {
		// Assert:
		AssertPassthroughForEmptyInput(consumer, disruptor::ConsumerInput());
	}

	// endregion
}}
