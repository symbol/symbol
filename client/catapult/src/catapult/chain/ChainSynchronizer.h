#pragma once
#include "NodeInteractionResult.h"
#include "catapult/disruptor/DisruptorTypes.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/thread/Future.h"
#include <functional>

namespace catapult {
	namespace api { class ChainApi; }
	namespace chain { struct RemoteApi; }
	namespace model { struct BlockChainConfiguration; }
}

namespace catapult { namespace chain {
	/// Function signature for synchronizing with a remote node
	using ChainSynchronizer = std::function<thread::future<NodeInteractionResult> (const RemoteApi&)>;

	/// Function signature for supplying a range of short hashes.
	using ShortHashesSupplier = std::function<model::ShortHashRange ()>;

	/// Function signature for delivering a block range to a consumer.
	using BlockRangeConsumerFunc = std::function<void (model::BlockRange&&)>;

	/// Function signature for delivering a block range to a consumer with an additional completion handler.
	using CompletionAwareBlockRangeConsumerFunc = std::function<disruptor::DisruptorElementId (
			model::BlockRange&&,
			const disruptor::ProcessingCompleteFunc&)>;

	/// Function signature for delivering a transaction range to a consumer.
	using TransactionRangeConsumerFunc = std::function<void (model::TransactionRange&&)>;

	/// Configuration for customizing a chain synchronizer.
	struct ChainSynchronizerConfiguration {
		/// The maximum number of blocks per sync attempt.
		uint32_t MaxBlocksPerSyncAttempt;

		/// The maximum chain bytes per sync attempt.
		uint32_t MaxChainBytesPerSyncAttempt;

		/// The maximum number of blocks that can be rolled back.
		uint32_t MaxRollbackBlocks;
	};

	/// Creates a chain synchronizer around the specified local chain api (\a pLocalChainApi), a block chain \a config,
	/// a short hashes supplier (\a shortHashesSupplier), a block range consumer (\a blockRangeConsumer) and a
	/// transaction range consumer (\a transactionRangeConsumer).
	ChainSynchronizer CreateChainSynchronizer(
			const std::shared_ptr<const api::ChainApi>& pLocalChainApi,
			const ChainSynchronizerConfiguration& config,
			const ShortHashesSupplier& shortHashesSupplier,
			const CompletionAwareBlockRangeConsumerFunc& blockRangeConsumer,
			const TransactionRangeConsumerFunc& transactionRangeConsumer);
}}
