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

#include "NemesisBlockHashesCalculator.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/chain/BlockExecutor.h"
#include "catapult/model/NemesisNotificationPublisher.h"
#include "catapult/observers/NotificationObserverAdapter.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace tools { namespace nemgen {

	BlockExecutionHashesInfo CalculateNemesisBlockExecutionHashes(
			const model::BlockElement& blockElement,
			const model::BlockChainConfiguration& config,
			plugins::PluginManager& pluginManager) {
		// 1. prepare observer
		auto publisherOptions = model::ExtractNemesisNotificationPublisherOptions(config);
		observers::NotificationObserverAdapter entityObserver(
				pluginManager.createObserver(),
				model::CreateNemesisNotificationPublisher(pluginManager.createNotificationPublisher(), publisherOptions));

		// 2. prepare observer state
		auto cache = pluginManager.createCache();
		auto cacheDetachableDelta = cache.createDetachableDelta();
		auto cacheDetachedDelta = cacheDetachableDelta.detach();
		auto pCacheDelta = cacheDetachedDelta.tryLock();
		auto blockStatementBuilder = model::BlockStatementBuilder();
		auto observerState = observers::ObserverState(*pCacheDelta, blockStatementBuilder);

		// 3. prepare resolvers
		auto readOnlyCache = pCacheDelta->toReadOnly();
		auto resolverContext = pluginManager.createResolverContext(readOnlyCache);

		// 4. execute block
		chain::ExecuteBlock(blockElement, { entityObserver, resolverContext, observerState });
		auto pBlockStatement = blockStatementBuilder.build();
		auto cacheStateHashInfo = pCacheDelta->calculateStateHash(blockElement.Block.Height);
		auto blockReceiptsHash = config.EnableVerifiableReceipts
				? model::CalculateMerkleHash(*pBlockStatement)
				: Hash256();

		return { blockReceiptsHash, cacheStateHashInfo.StateHash, cacheStateHashInfo.SubCacheMerkleRoots, std::move(pBlockStatement) };
	}
}}}
