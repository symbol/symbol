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
#include "catapult/types.h"
#include <vector>

namespace catapult {
	namespace cache { class CatapultCache; }
	namespace chain { struct ExecutionConfiguration; }
	namespace model {
		struct Block;
		struct BlockChainConfiguration;
	}
}

namespace catapult { namespace harvesting {

	/// Block hashes dependent on block execution.
	struct BlockExecutionHashes {
	public:
		/// Creates block hashes around \a isExecutionSuccess.
		explicit BlockExecutionHashes(bool isExecutionSuccess)
				: IsExecutionSuccess(isExecutionSuccess)
				, ReceiptsHash()
				, StateHash()
		{}

	public:
		/// \c true if block execution succeeded.
		bool IsExecutionSuccess;

		/// Block receipts hash.
		Hash256 ReceiptsHash;

		/// Block state hash.
		Hash256 StateHash;
	};

	/// Calculates the block execution dependent hashes after executing \a block with specified transaction hashes (\a transactionHashes)
	/// given \a cache for the network configured with \a config and \a executionConfig.
	BlockExecutionHashes CalculateBlockExecutionHashes(
			const model::Block& block,
			const std::vector<Hash256>& transactionHashes,
			const cache::CatapultCache& cache,
			const model::BlockChainConfiguration& config,
			const chain::ExecutionConfiguration& executionConfig);
}}
