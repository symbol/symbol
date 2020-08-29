/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#pragma once
#include "RemoteNodeSynchronizer.h"
#include "catapult/disruptor/DisruptorTypes.h"
#include "catapult/model/AnnotatedEntityRange.h"
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
			model::AnnotatedBlockRange&&,
			const disruptor::ProcessingCompleteFunc&)>;

	/// Configuration for customizing a chain synchronizer.
	struct ChainSynchronizerConfiguration {
		/// Maximum number of hashes per sync attempt.
		uint32_t MaxHashesPerSyncAttempt;

		/// Maximum number of blocks per sync attempt.
		uint32_t MaxBlocksPerSyncAttempt;

		/// Maximum chain bytes per sync attempt.
		uint32_t MaxChainBytesPerSyncAttempt;

		/// Maximum number of blocks that can be rolled back.
		uint32_t MaxRollbackBlocks;
	};

	/// Creates a chain synchronizer around the specified local chain api (\a pLocalChainApi), block chain \a config,
	/// local finalized height supplier (\a localFinalizedHeightSupplier) and block range consumer (\a blockRangeConsumer).
	RemoteNodeSynchronizer<api::RemoteChainApi> CreateChainSynchronizer(
			const std::shared_ptr<const api::ChainApi>& pLocalChainApi,
			const ChainSynchronizerConfiguration& config,
			const supplier<Height>& localFinalizedHeightSupplier,
			const CompletionAwareBlockRangeConsumerFunc& blockRangeConsumer);
}}
