#pragma once
#include "RemoteNodeSynchronizer.h"
#include "catapult/disruptor/DisruptorTypes.h"
#include "catapult/model/RangeTypes.h"

namespace catapult {
	namespace api {
		class ChainApi;
		class RemoteChainApi;
	}
	namespace model { struct BlockChainConfiguration; }
}

namespace catapult { namespace chain {

	/// Function signature for delivering a block range to a consumer.
	using BlockRangeConsumerFunc = consumer<model::BlockRange&&>;

	/// Function signature for delivering a block range to a consumer with an additional completion handler.
	using CompletionAwareBlockRangeConsumerFunc = std::function<disruptor::DisruptorElementId (
			model::BlockRange&&,
			const disruptor::ProcessingCompleteFunc&)>;

	/// Configuration for customizing a chain synchronizer.
	struct ChainSynchronizerConfiguration {
		/// The maximum number of blocks per sync attempt.
		uint32_t MaxBlocksPerSyncAttempt;

		/// The maximum chain bytes per sync attempt.
		uint32_t MaxChainBytesPerSyncAttempt;

		/// The maximum number of blocks that can be rolled back.
		uint32_t MaxRollbackBlocks;
	};

	/// Creates a chain synchronizer around the specified local chain api (\a pLocalChainApi), a block chain \a config and
	/// a block range consumer (\a blockRangeConsumer).
	RemoteNodeSynchronizer<api::RemoteChainApi> CreateChainSynchronizer(
			const std::shared_ptr<const api::ChainApi>& pLocalChainApi,
			const ChainSynchronizerConfiguration& config,
			const CompletionAwareBlockRangeConsumerFunc& blockRangeConsumer);
}}
