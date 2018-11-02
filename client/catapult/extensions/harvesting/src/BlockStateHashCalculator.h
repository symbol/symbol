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

namespace catapult {
	namespace cache { class CatapultCache; }
	namespace model {
		struct Block;
		struct BlockChainConfiguration;
	}
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace harvesting {

	/// Calculates the state hash after executing \a block given \a cache for the network configured with \a config
	/// and \a pluginManager.
	std::pair<Hash256, bool> CalculateBlockStateHash(
			const model::Block& block,
			const cache::CatapultCache& cache,
			const model::BlockChainConfiguration& config,
			const plugins::PluginManager& pluginManager);
}}
