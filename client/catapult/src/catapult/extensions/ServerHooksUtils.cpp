#include "ServerHooksUtils.h"

namespace catapult { namespace extensions {

	namespace {
		template<typename TConsumerFactory>
		auto CreatePushEntityCallback(const TConsumerFactory& consumerFactory, const extensions::ChainSyncedPredicate& isChainSynced) {
			return [isChainSynced, consumer = consumerFactory(disruptor::InputSource::Remote_Push)](auto&& range) {
				if (isChainSynced())
					consumer(std::move(range));
			};
		}
	}

	BlockRangeConsumerFunc CreateBlockPushEntityCallback(const ServerHooks& hooks) {
		return CreatePushEntityCallback(hooks.blockRangeConsumerFactory(), hooks.chainSyncedPredicate());
	}

	TransactionRangeConsumerFunc CreateTransactionPushEntityCallback(const ServerHooks& hooks) {
		return CreatePushEntityCallback(hooks.transactionRangeConsumerFactory(), hooks.chainSyncedPredicate());
	}
}}
