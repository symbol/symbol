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
					CATAPULT_LOG(error) << "error when loading state, remove state directory and start again";
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
				auto pObserver = extensions::CreateEntityObserver(pluginManager);
				auto pPermanentObserver = extensions::CreatePermanentEntityObserver(pluginManager);
				auto pPublisher = pluginManager.createNotificationPublisher();

				auto storageView = stateRef.Storage.view();
				auto pLastBlock = storageView.loadBlock(storageView.chainHeight());
				auto observerFactory = CreateBlockDependentEntityObserverFactory(
						*pLastBlock,
						stateRef.Config.BlockChain,
						*pObserver,
						*pPermanentObserver);

				auto pNemesisBlock = storageView.loadBlock(Height(1));
				extensions::NemesisBlockLoader loader(pluginManager.transactionRegistry(), *pPublisher, observerFactory(*pNemesisBlock));
				loader.executeAndCommit(stateRef);

				loadBlockChainFromStorage(observerFactory, stateRef, Height(2));
			}

			void loadPartialBlockChainFromStorage(
					const extensions::LocalNodeStateRef& stateRef,
					const plugins::PluginManager& pluginManager,
					Height startHeight) {
				// disable load optimizations (loading from the saved state is optimization enough) in order to prevent
				// discontinuities in block analysis (e.g. difficulty cache expects consecutive blocks)
				auto pObserver = extensions::CreateEntityObserver(pluginManager);
				const auto& observer = *pObserver;
				auto observerFactory = [&observer](const auto&) -> const observers::EntityObserver& {
					return observer;
				};

				loadBlockChainFromStorage(observerFactory, stateRef, startHeight);
			}

			void loadBlockChainFromStorage(
					const BlockDependentEntityObserverFactory& observerFactory,
					const extensions::LocalNodeStateRef& stateRef,
					Height startHeight) {
				auto score = LoadBlockChain(observerFactory, stateRef, startHeight);
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
