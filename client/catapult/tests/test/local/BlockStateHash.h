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

namespace catapult {
	namespace cache { class CatapultCacheDelta; }
	namespace config { class CatapultConfiguration; }
	namespace model {
		struct Block;
		struct BlockElement;
	}
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace test {

	/// Calculate nemesis state hash of \a blockElement given \a config.
	Hash256 CalculateNemesisStateHash(const model::BlockElement& blockElement, const config::CatapultConfiguration& config);

	/// Calculates the state hash after executing \a block given \a cache for the network configured with \a pluginManager.
	Hash256 CalculateBlockStateHash(
			const model::Block& block,
			cache::CatapultCacheDelta& cache,
			const plugins::PluginManager& pluginManager);
}}
