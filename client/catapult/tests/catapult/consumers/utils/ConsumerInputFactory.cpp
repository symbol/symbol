#include "ConsumerInputFactory.h"
#include "ConsumerTestUtils.h"
#include "catapult/consumers/BlockConsumers.h"
#include "catapult/consumers/TransactionConsumers.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"

using catapult::disruptor::ConsumerInput;

namespace catapult { namespace test {

	namespace {
		ConsumerInput PrepareBlockInput(ConsumerInput&& input) {
			// 1. link all blocks
			LinkBlocks(Height(12), input.blocks());

			// 2. add all hashes
			auto pTransactionRegistry = mocks::CreateDefaultTransactionRegistry();
			consumers::CreateBlockHashCalculatorConsumer(*pTransactionRegistry)(input.blocks());
			return std::move(input);
		}

		ConsumerInput PrepareTransactionInput(ConsumerInput&& input) {
			// 1. add all hashes
			auto pTransactionRegistry = mocks::CreateDefaultTransactionRegistry();
			consumers::CreateTransactionHashCalculatorConsumer(*pTransactionRegistry)(input.transactions());
			return std::move(input);
		}
	}

	ConsumerInput CreateConsumerInputWithBlocks(size_t numBlocks, disruptor::InputSource source) {
		auto range = CreateBlockEntityRange(numBlocks);
		return PrepareBlockInput(ConsumerInput(std::move(range), source));
	}

	ConsumerInput CreateConsumerInputWithTransactions(size_t numTransactions, disruptor::InputSource source) {
		auto range = CreateTransactionEntityRange(numTransactions);
		return PrepareTransactionInput(ConsumerInput(std::move(range), source));
	}

	ConsumerInput CreateConsumerInputFromBlocks(const std::vector<const model::Block*>& blocks) {
		auto range = CreateEntityRange(blocks);
		return PrepareBlockInput(ConsumerInput(std::move(range)));
	}

	ConsumerInput CreateConsumerInputFromTransactions(const std::vector<const model::Transaction*>& transactions) {
		auto range = CreateEntityRange(transactions);
		return PrepareTransactionInput(ConsumerInput(std::move(range)));
	}
}}
