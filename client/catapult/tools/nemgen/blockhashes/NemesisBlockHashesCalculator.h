/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include <memory>
#include <vector>

namespace catapult {
	namespace model {
		struct BlockchainConfiguration;
		struct BlockElement;
		struct BlockStatement;
	}
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace tools { namespace nemgen {

	/// Information about block execution dependent hashes.
	struct BlockExecutionHashesInfo {
		/// Number of voting eligible accounts.
		uint32_t VotingEligibleAccountsCount;

		/// Number of harvesting eligible accounts.
		uint64_t HarvestingEligibleAccountsCount;

		/// Total balance eligible for voting.
		Amount TotalVotingBalance;

		/// Block receipts hash.
		Hash256 ReceiptsHash;

		/// Block state hash.
		Hash256 StateHash;

		/// Block component sub cache merkle roots.
		std::vector<Hash256> SubCacheMerkleRoots;

		/// Block statement.
		std::unique_ptr<model::BlockStatement> pBlockStatement;
	};

	/// Calculates the block execution dependent hashes after executing nemesis \a blockElement for network configured with \a config
	/// and \a pluginManager.
	BlockExecutionHashesInfo CalculateNemesisBlockExecutionHashes(
			const model::BlockElement& blockElement,
			const model::BlockchainConfiguration& config,
			plugins::PluginManager& pluginManager);
}}}
