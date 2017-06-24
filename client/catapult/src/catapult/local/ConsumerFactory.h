#pragma once
#include "catapult/chain/ChainSynchronizer.h"
#include "catapult/disruptor/DisruptorElement.h"

namespace catapult { namespace local {

	/// Factory for creating a BlockRangeConsumerFunc bound to an input source.
	using BlockRangeConsumerFactoryFunc =
			std::function<chain::BlockRangeConsumerFunc (disruptor::InputSource)>;

	/// Factory for creating a CompletionAwareBlockRangeConsumerFunc bound to an input source.
	using CompletionAwareBlockRangeConsumerFactoryFunc =
			std::function<chain::CompletionAwareBlockRangeConsumerFunc (disruptor::InputSource)>;

	/// Factory for creating a TransactionRangeConsumerFunc bound to an input source.
	using TransactionRangeConsumerFactoryFunc = std::function<chain::TransactionRangeConsumerFunc (disruptor::InputSource)>;
}}
