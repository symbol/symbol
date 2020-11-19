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

#include "ConsumerTestUtils.h"
#include "ConsumerInputFactory.h"
#include "sdk/src/extensions/BlockExtensions.h"
#include "catapult/model/EntityHasher.h"
#include "tests/test/core/TransactionTestUtils.h"
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
		auto blockExtensions = extensions::BlockExtensions(GetDefaultGenerationHashSeed());
		auto* pParentBlock = const_cast<model::Block*>(&blockElements[0].Block);
		blockExtensions.updateBlockTransactionsHash(*pParentBlock);
		pParentBlock->Height = chainHeight;
		pParentBlock->Timestamp = static_cast<Timestamp>(chainHeight.unwrap() * 2);

		for (auto i = 1u; i < blockElements.size(); ++i) {
			auto& block = const_cast<model::Block&>(blockElements[i].Block);
			blockExtensions.updateBlockTransactionsHash(block);
			LinkBlocks(*pParentBlock, block);
			pParentBlock = &block;
		}
	}

	// endregion

	// region ConsumerResult Assertions

	void AssertConsumed(const disruptor::ConsumerResult& result, validators::ValidationResult validationResult) {
		auto expectedSeverity = validators::ValidationResult::Success == validationResult
				? disruptor::ConsumerResultSeverity::Success
				: disruptor::ConsumerResultSeverity::Neutral;
		EXPECT_EQ(disruptor::CompletionStatus::Consumed, result.CompletionStatus);
		EXPECT_EQ(validationResult, static_cast<validators::ValidationResult>(result.CompletionCode));
		EXPECT_EQ(expectedSeverity, result.ResultSeverity);
	}

	void AssertAborted(
			const disruptor::ConsumerResult& result,
			validators::ValidationResult validationResult,
			disruptor::ConsumerResultSeverity severity) {
		EXPECT_EQ(disruptor::CompletionStatus::Aborted, result.CompletionStatus);
		EXPECT_EQ(validationResult, static_cast<validators::ValidationResult>(result.CompletionCode));
		EXPECT_EQ(severity, result.ResultSeverity);
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
			test::AssertAborted(result, consumers::Failure_Consumer_Empty_Input, disruptor::ConsumerResultSeverity::Failure);
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
