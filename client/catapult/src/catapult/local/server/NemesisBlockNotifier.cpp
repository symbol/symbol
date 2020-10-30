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

#include "NemesisBlockNotifier.h"
#include "catapult/cache/CacheChanges.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/extensions/NemesisBlockLoader.h"
#include "catapult/io/BlockChangeSubscriber.h"
#include "catapult/io/BlockStatementSerializer.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/model/ChainScore.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/subscribers/FinalizationSubscriber.h"
#include "catapult/subscribers/StateChangeInfo.h"
#include "catapult/subscribers/StateChangeSubscriber.h"

namespace catapult { namespace local {

	namespace {
		constexpr auto Nemesis_Height = Height(1);
		constexpr auto Nemesis_Chain_Score = static_cast<uint64_t>(1);

		bool HasPreviousExecution(const cache::CatapultCache& cache) {
			// assume a previous execution if the account state cache is not empty
			// (this needs to be checked in order to prevent reexecution, which can lead to hash cache insert error)
			return 0 != cache.createView().sub<cache::AccountStateCache>().size();
		}
	}

	NemesisBlockNotifier::NemesisBlockNotifier(
			const model::BlockChainConfiguration& config,
			const cache::CatapultCache& cache,
			const io::BlockStorageCache& storage,
			const plugins::PluginManager& pluginManager)
			: m_config(config)
			, m_cache(cache)
			, m_storage(storage)
			, m_pluginManager(pluginManager)
	{}

	void NemesisBlockNotifier::raise(io::BlockChangeSubscriber& subscriber) {
		raise([&subscriber](const auto& nemesisBlockElement) {
			subscriber.notifyBlock(nemesisBlockElement);
		});
	}

	void NemesisBlockNotifier::raise(subscribers::FinalizationSubscriber& subscriber) {
		raise([&subscriber](const auto& nemesisBlockElement) {
			subscriber.notifyFinalizedBlock({ FinalizationEpoch(1), FinalizationPoint(1) }, Height(1), nemesisBlockElement.EntityHash);
		});
	}

	void NemesisBlockNotifier::raise(subscribers::StateChangeSubscriber& subscriber) {
		raise([&subscriber, &config = m_config, &cache = m_cache, &pluginManager = m_pluginManager](const auto& nemesisBlockElement) {
			// execute the nemesis block
			auto cacheDetachableDelta = cache.createDetachableDelta();
			auto cacheDetachedDelta = cacheDetachableDelta.detach();
			auto pCacheDelta = cacheDetachedDelta.tryLock();

			extensions::NemesisBlockLoader loader(*pCacheDelta, pluginManager, pluginManager.createObserver());
			loader.execute(config, nemesisBlockElement);

			// notify nemesis cache state
			subscriber.notifyScoreChange(model::ChainScore(Nemesis_Chain_Score));
			subscriber.notifyStateChange({
				cache::CacheChanges(*pCacheDelta),
				model::ChainScore::Delta(Nemesis_Chain_Score),
				Nemesis_Height
			});
		});
	}

	void NemesisBlockNotifier::raise(const consumer<model::BlockElement>& action) {
		// bypass if chain has advanced past nemesis and/or appears to have been previously executed
		auto storageView = m_storage.view();
		if (Nemesis_Height != storageView.chainHeight() || HasPreviousExecution(m_cache))
			CATAPULT_THROW_RUNTIME_ERROR("NemesisBlockNotifier can only be called during first boot");

		auto pNemesisBlockElement = storageView.loadBlockElement(Nemesis_Height);
		auto nemesisBlockStatementPair = storageView.loadBlockStatementData(Nemesis_Height);
		if (nemesisBlockStatementPair.second) {
			auto pNemesisBlockStatement = std::make_shared<model::BlockStatement>();
			io::BufferInputStreamAdapter<std::vector<uint8_t>> nemesisBlockStatementStream(nemesisBlockStatementPair.first);
			io::ReadBlockStatement(nemesisBlockStatementStream, *pNemesisBlockStatement);
			const_cast<model::BlockElement&>(*pNemesisBlockElement).OptionalStatement = std::move(pNemesisBlockStatement);
		}

		action(*pNemesisBlockElement);
	}
}}
