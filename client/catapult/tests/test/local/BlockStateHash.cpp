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

#include "BlockStateHash.h"
#include "LocalTestUtils.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/chain/BlockExecutor.h"
#include "catapult/observers/NotificationObserverAdapter.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/Nemesis.h"

namespace catapult { namespace test {

	Hash256 CalculateNemesisStateHash(const model::BlockElement& blockElement, const config::CatapultConfiguration& config) {
		auto pPluginManager = CreatePluginManagerWithRealPlugins(config);

		auto cache = pPluginManager->createCache();
		auto cacheDetachableDelta = cache.createDetachableDelta();
		auto cacheDetachedDelta = cacheDetachableDelta.detach();
		auto pCacheDelta = cacheDetachedDelta.tryLock();

		return CalculateBlockStateHash(blockElement.Block, *pCacheDelta, *pPluginManager);
	}

	Hash256 CalculateBlockStateHash(
			const model::Block& block,
			cache::CatapultCacheDelta& cache,
			const plugins::PluginManager& pluginManager) {
		// 1. prepare observer
		observers::NotificationObserverAdapter entityObserver(pluginManager.createObserver(), pluginManager.createNotificationPublisher());

		// 2. prepare observer state
		auto observerState = observers::ObserverState(cache);

		// 3. prepare resolvers
		auto readOnlyCache = cache.toReadOnly();
		auto resolverContext = pluginManager.createResolverContext(readOnlyCache);

		// 4. execute block
		chain::ExecuteBlock(
				BlockToBlockElement(block, GetNemesisGenerationHashSeed()),
				{ entityObserver, resolverContext, observerState });
		return cache.calculateStateHash(block.Height).StateHash;
	}
}}
