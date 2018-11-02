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

#include "StateHashCalculator.h"
#include "PluginLoader.h"
#include "catapult/chain/BlockExecutor.h"
#include "catapult/observers/NotificationObserverAdapter.h"

namespace catapult { namespace tools { namespace nemgen {

	BlockStateHashInfo CalculateNemesisStateHash(const model::BlockElement& blockElement, const config::LocalNodeConfiguration& config) {
		// 1. load all plugins
		PluginLoader pluginLoader(config);
		pluginLoader.loadAll();
		auto& pluginManager = pluginLoader.manager();

		// 2. prepare observer
		observers::NotificationObserverAdapter entityObserver(pluginManager.createObserver(), pluginManager.createNotificationPublisher());

		// 3. prepare observer state
		auto cache = pluginManager.createCache();
		auto cacheDetachedDelta = cache.createDetachableDelta().detach();
		auto pCacheDelta = cacheDetachedDelta.lock();
		auto catapultState = state::CatapultState();
		auto observerState = observers::ObserverState(*pCacheDelta, catapultState);

		// 4. execute block
		chain::ExecuteBlock(blockElement, entityObserver, observerState);
		auto cacheStateHashInfo = pCacheDelta->calculateStateHash(blockElement.Block.Height);
		return { cacheStateHashInfo.StateHash, cacheStateHashInfo.SubCacheMerkleRoots };
	}
}}}
