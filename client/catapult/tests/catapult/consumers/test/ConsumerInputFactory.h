#pragma once
#include "catapult/disruptor/DisruptorConsumer.h"
#include "catapult/disruptor/DisruptorElement.h"

namespace catapult { namespace test {

	/// Creates a consumer input with \a numBlocks top-level blocks with \a source.
	disruptor::ConsumerInput CreateConsumerInputWithBlocks(size_t numBlocks, disruptor::InputSource source);

	/// Creates a consumer input with \a numTransactions top-level transactions with \a source.
	disruptor::ConsumerInput CreateConsumerInputWithTransactions(size_t numTransactions, disruptor::InputSource source);

	/// Creates a consumer input composed of the specified \a blocks.
	disruptor::ConsumerInput CreateConsumerInputFromBlocks(const std::vector<const model::Block*>& blocks);

	/// Creates a consumer input composed of the specified \a transactions.
	disruptor::ConsumerInput CreateConsumerInputFromTransactions(const std::vector<const model::Transaction*>& transactions);
}}
