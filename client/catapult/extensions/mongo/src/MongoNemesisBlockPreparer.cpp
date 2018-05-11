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

#include "MongoNemesisBlockPreparer.h"
#include "ExternalCacheStorage.h"
#include "MongoBlockStorageUtils.h"
#include "catapult/extensions/NemesisBlockLoader.h"
#include "catapult/extensions/PluginUtils.h"
#include "catapult/io/BlockStorage.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace mongo {

	MongoNemesisBlockPreparer::MongoNemesisBlockPreparer(
			io::LightBlockStorage& destinationStorage,
			ExternalCacheStorage& externalCacheStorage,
			const model::BlockChainConfiguration& config,
			const io::BlockStorage& sourceStorage,
			const plugins::PluginManager& pluginManager)
			: m_destinationStorage(destinationStorage)
			, m_externalCacheStorage(externalCacheStorage)
			, m_config(config)
			, m_sourceStorage(sourceStorage)
			, m_pluginManager(pluginManager)
	{}

	bool MongoNemesisBlockPreparer::prepare(const cache::CatapultCache& cache) const {
		auto height = m_destinationStorage.chainHeight();
		if (Height() != height) {
			CATAPULT_LOG(debug) << "storage is already initialized with height " << height;
			return false;
		}

		// 1. prepare the nemesis storage by computing derived properties
		auto pNotificationPublisher = m_pluginManager.createNotificationPublisher();
		PrepareMongoBlockStorage(m_destinationStorage, m_sourceStorage, *pNotificationPublisher);

		// 2. create a detached delta
		auto cacheDetachedDelta = cache.createDetachableDelta().detach();
		auto pCacheDelta = cacheDetachedDelta.lock();

		// 3. execute the nemesis block
		auto pEntityObserver = extensions::CreateEntityObserver(m_pluginManager);
		auto pNemesisBlockElement = m_sourceStorage.loadBlockElement(Height(1));
		extensions::NemesisBlockLoader loader(m_pluginManager.transactionRegistry(), *pNotificationPublisher, *pEntityObserver);
		loader.execute(m_config, *pNemesisBlockElement, *pCacheDelta);

		// 4. save the cache state externally (into mongo)
		m_externalCacheStorage.saveDelta(*pCacheDelta);
		return true;
	}
}}
