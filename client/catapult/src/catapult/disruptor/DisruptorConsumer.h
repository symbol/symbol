#pragma once
#include "DisruptorElement.h"
#include <functional>

namespace catapult { namespace disruptor {

	/// A typed disruptor consumer function.
	template<typename TInput>
	using DisruptorConsumerT = std::function<ConsumerResult (TInput&)>;

	/// A disruptor consumer function.
	using DisruptorConsumer = DisruptorConsumerT<ConsumerInput>;

	/// A const disruptor consumer function.
	using ConstDisruptorConsumer = DisruptorConsumerT<const ConsumerInput>;

	/// A block disruptor consumer function.
	using BlockConsumer = DisruptorConsumerT<BlockElements>;

	/// A const block disruptor consumer function.
	using ConstBlockConsumer = DisruptorConsumerT<const BlockElements>;

	/// A transaction disruptor consumer function.
	using TransactionConsumer = DisruptorConsumerT<TransactionElements>;

	/// A const transaction disruptor consumer function.
	using ConstTransactionConsumer = DisruptorConsumerT<const TransactionElements>;

	/// Maps \a blockConsumers to disruptor consumers so that they can be used to create a ConsumerDispatcher.
	std::vector<DisruptorConsumer> DisruptorConsumersFromBlockConsumers(
			const std::vector<BlockConsumer>& blockConsumers);

	/// Maps \a transactionConsumers to disruptor consumers so that they can be used to create a ConsumerDispatcher.
	std::vector<DisruptorConsumer> DisruptorConsumersFromTransactionConsumers(
			const std::vector<TransactionConsumer>& transactionConsumers);
}}
