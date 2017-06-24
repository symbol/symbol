#include "DisruptorConsumer.h"

namespace catapult { namespace disruptor {

	namespace {
		template<typename TConsumer, typename TInvoker>
		std::vector<DisruptorConsumer> DisruptorConsumersFromTypedConsumers(
				const std::vector<TConsumer>& typedConsumers,
				TInvoker typedConsumerInvoker) {
			if (typedConsumers.empty())
				CATAPULT_THROW_INVALID_ARGUMENT("no consumers were specified");

			std::vector<DisruptorConsumer> consumers;
			for (const auto& typedConsumer : typedConsumers) {
				consumers.push_back([typedConsumer, typedConsumerInvoker](auto& input) {
					return typedConsumerInvoker(typedConsumer, input);
				});
			}

			return consumers;
		}
	}

	std::vector<DisruptorConsumer> DisruptorConsumersFromBlockConsumers(const std::vector<BlockConsumer>& blockConsumers) {
		return DisruptorConsumersFromTypedConsumers(
				blockConsumers,
				[](const auto& blockConsumer, auto& input) { return blockConsumer(input.blocks()); });
	}

	std::vector<DisruptorConsumer> DisruptorConsumersFromTransactionConsumers(
			const std::vector<TransactionConsumer>& transactionConsumers) {
		return DisruptorConsumersFromTypedConsumers(
				transactionConsumers,
				[](const auto& transactionConsumer, auto& input) { return transactionConsumer(input.transactions()); });
	}
}}
