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

#include "FileBlockChainStorage.h"
#include "LocalNodeStateStorage.h"
#include "MultiBlockLoader.h"
#include "catapult/cache/SupplementalData.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/extensions/LocalNodeStateRef.h"
#include "catapult/extensions/NemesisBlockLoader.h"
#include "catapult/extensions/PluginUtils.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace filechain {

	namespace {
		void LogChainStats(const char* source, Height height, const model::ChainScore& score) {
			CATAPULT_LOG(info) << "loaded block chain from " << source << " (height = " << height << ", score = " << score << ")";
		}

		class FileBlockChainStorage : public extensions::BlockChainStorage {
		public:
			void loadFromStorage(const extensions::LocalNodeStateRef& stateRef, const plugins::PluginManager& pluginManager) override {
				cache::SupplementalData supplementalData;
				bool isStateLoaded = false;
				try {
					isStateLoaded = LoadState(stateRef.Config.User.DataDirectory, stateRef.Cache, supplementalData);
				} catch (...) {
					CATAPULT_LOG(error) << "error when loading state, remove state directories and start again";
					throw;
				}

				// if state is not present, load full chain from storage
				auto storageHeight = stateRef.Storage.view().chainHeight();
				if (!isStateLoaded) {
					loadCompleteBlockChainFromStorage(stateRef, pluginManager);
					LogChainStats("block storage", storageHeight, stateRef.Score.get());
					return;
				}

				// otherwise, use loaded state
				stateRef.Score += supplementalData.ChainScore;
				stateRef.State = supplementalData.State;

				auto cacheHeight = stateRef.Cache.createView().height();
				LogChainStats("state", cacheHeight, stateRef.Score.get());

				// if there are any additional storage blocks, load them too
				if (storageHeight <= cacheHeight)
					return;

				loadPartialBlockChainFromStorage(stateRef, pluginManager, cacheHeight + Height(1));
				LogChainStats("state and block storage", storageHeight, stateRef.Score.get());
			}

		private:
			void loadCompleteBlockChainFromStorage(
					const extensions::LocalNodeStateRef& stateRef,
					const plugins::PluginManager& pluginManager) {
				auto pPublisher = pluginManager.createNotificationPublisher();

				auto storageView = stateRef.Storage.view();
				auto pLastBlock = storageView.loadBlock(storageView.chainHeight());
				auto observerFactory = CreateBlockDependentNotificationObserverFactory(
						*pLastBlock,
						stateRef.Config.BlockChain,
						[&pluginManager]() { return pluginManager.createObserver(); },
						[&pluginManager]() { return pluginManager.createPermanentObserver(); });

				auto pNemesisBlock = storageView.loadBlock(Height(1));

				// if verifiable state is enabled check it by executing only non-permanent observers on detached delta
				if (stateRef.Config.BlockChain.ShouldEnableVerifiableState) {
					auto detachedDelta = stateRef.Cache.createDetachableDelta().detach();
					auto pCacheDelta = detachedDelta.lock();
					auto& cacheDelta = *pCacheDelta;

					extensions::NemesisBlockLoader loader(cacheDelta, pluginManager, pluginManager.createObserver());

					state::CatapultState temporaryState;
					extensions::LocalNodeStateRef stateRefCopy(
							stateRef.Config,
							temporaryState,
							stateRef.Cache,
							stateRef.Storage,
							stateRef.Score);

					loader.execute(stateRefCopy, extensions::StateHashVerification::Enabled);
				}

				// if verifiable state is enabled do NOT check hash when committing.
				{
					auto cacheDelta = stateRef.Cache.createDelta();
					extensions::NemesisBlockLoader loader(cacheDelta, pluginManager, observerFactory(*pNemesisBlock));

					auto stateHashVerification = stateRef.Config.BlockChain.ShouldEnableVerifiableState
							? extensions::StateHashVerification::Disabled
							: extensions::StateHashVerification::Enabled;

					loader.executeAndCommit(stateRef, stateHashVerification);
				}

				loadBlockChainFromStorage(observerFactory, pluginManager, stateRef, Height(2));
			}

			void loadPartialBlockChainFromStorage(
					const extensions::LocalNodeStateRef& stateRef,
					const plugins::PluginManager& pluginManager,
					Height startHeight) {
				if (stateRef.Config.Node.ShouldUseCacheDatabaseStorage)
					CATAPULT_THROW_RUNTIME_ERROR("cannot load partial block chain from storage when cache database is enabled");

				// disable load optimizations (loading from the saved state is optimization enough) in order to prevent
				// discontinuities in block analysis (e.g. difficulty cache expects consecutive blocks)
				auto observerFactory = [&pluginManager](const auto&) { return pluginManager.createObserver(); };
				loadBlockChainFromStorage(observerFactory, pluginManager, stateRef, startHeight);
			}

			void loadBlockChainFromStorage(
					const BlockDependentNotificationObserverFactory& observerFactory,
					const plugins::PluginManager& pluginManager,
					const extensions::LocalNodeStateRef& stateRef,
					Height startHeight) {
				auto score = LoadBlockChain(observerFactory, pluginManager, stateRef, startHeight);
				stateRef.Score += score;
			}

		public:
			void saveToStorage(const extensions::LocalNodeStateConstRef& stateRef) override {
				SaveState(stateRef.Config.User.DataDirectory, stateRef.Cache, { stateRef.State, stateRef.Score.get() });
			}
		};
	}

	std::unique_ptr<extensions::BlockChainStorage> CreateFileBlockChainStorage() {
		return std::make_unique<FileBlockChainStorage>();
	}
}}
