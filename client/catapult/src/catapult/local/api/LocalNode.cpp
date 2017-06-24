#include "LocalNode.h"
#include "ChainScoreProvider.h"
#include "DispatcherService.h"
#include "LocalNodeApiState.h"
#include "NetworkPacketReaderService.h"
#include "NetworkPacketWriterService.h"
#include "SchedulerService.h"
#include "plugins/mongo/coremongo/src/ExternalCacheStorage.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/SupplementalData.h"
#include "catapult/cache/UtCache.h"
#include "catapult/consumers/StateChangeInfo.h"
#include "catapult/io/FileBasedStorage.h"
#include "catapult/local/BasicLocalNode.h"
#include "catapult/local/NemesisBlockLoader.h"
#include "catapult/local/PluginUtils.h"

namespace catapult { namespace local { namespace api {

	namespace {
		class DefaultLocalNode final : public BasicLocalNode {
		public:
			DefaultLocalNode(
					config::LocalNodeConfiguration&& config,
					std::unique_ptr<thread::MultiServicePool>&& pMultiServicePool,
					std::unique_ptr<io::BlockStorage>&& pBlockStorage,
					std::shared_ptr<ChainScoreProvider>&& pChainScoreProvider,
					std::unique_ptr<mongo::plugins::ExternalCacheStorage>&& pCacheStorage,
					const UnconfirmedTransactionsCacheViewProvider& unconfirmedTransactionsCacheViewProvider,
					std::unique_ptr<cache::UtCache>&& pUnconfirmedTransactionCache,
					const crypto::KeyPair& keyPair)
					: BasicLocalNode(std::move(config), std::move(pMultiServicePool), std::move(pBlockStorage))
					, m_pChainScoreProvider(std::move(pChainScoreProvider))
					, m_pCacheStorage(std::move(pCacheStorage))
					, m_unconfirmedTransactionsCacheViewProvider(unconfirmedTransactionsCacheViewProvider)
					, m_pUnconfirmedTransactionCache(std::move(pUnconfirmedTransactionCache))
					, m_apiState(stateRef())
					, m_packetWriterService(keyPair, stateCref().Config)
					, m_dispatcherService(
							m_apiState,
							pluginManager(),
							m_unconfirmedTransactionsCacheViewProvider,
							*m_pUnconfirmedTransactionCache,
							m_packetWriterService.createNewTransactionsSink())
					, m_packetReaderService(
							keyPair,
							stateCref(),
							pluginManager().transactionRegistry(),
							m_dispatcherService.createTransactionRangeConsumerFactory()(disruptor::InputSource::Remote_Push))
					, m_schedulerService(
							stateCref(),
							pluginManager().transactionRegistry(),
							m_unconfirmedTransactionsCacheViewProvider,
							m_dispatcherService.createCompletionAwareBlockRangeConsumerFactory()) {
				m_apiState.subscribeStateChange([&chainScoreProvider = *m_pChainScoreProvider, &cacheStorage = *m_pCacheStorage](
						const auto& newScore,
						const consumers::StateChangeInfo& stateChangeInfo) {
					chainScoreProvider.saveScore(newScore);
					cacheStorage.saveDelta(stateChangeInfo.CacheDelta);
				});
			}

			~DefaultLocalNode() {
				shutdown();
			}

		private:
			void registerPlugins(const RegisterPluginFunc&) override {
				// no additional plugins required for api node
			}

			void copyNemesisFromDiskToStorage() {
				auto stateRef = this->stateRef();
				io::FileBasedStorage fileStorage(stateRef.Config.User.DataDirectory);
				auto pNemesisBlock = fileStorage.loadBlockElement(Height(1));
				auto modifier = stateRef.Storage.modifier();
				modifier.saveBlock(*pNemesisBlock);
			}

			bool promoteNemesisBlockToStorage() {
				auto stateRef = this->stateRef();
				auto height = stateRef.Storage.view().chainHeight();
				if (Height(0) != height) {
					CATAPULT_LOG(debug) << "storage is already initialized with height " << height;
					return false;
				}

				CATAPULT_LOG(info) << "initializing storage with nemesis data";

				// 1. save the nemesis block into storage
				copyNemesisFromDiskToStorage();

				// 2. load the nemesis block into memory
				auto pEntityObserver = CreateEntityObserver(pluginManager());
				LoadNemesisBlock(
						pluginManager().transactionRegistry(),
						*pEntityObserver,
						stateRef,
						[&storage = *m_pCacheStorage](auto& cacheDelta) {
							storage.saveDelta(cacheDelta);
						});

				return true;
			}

			void loadFromStorage() override {
				if (promoteNemesisBlockToStorage())
					return;

				cache::SupplementalData supplementalData;
				auto stateRef = this->stateRef();
				auto chainHeight = stateRef.Storage.view().chainHeight();
				m_pCacheStorage->loadAll(stateRef.Cache, chainHeight);

				// TODO - loadScore/saveScore should probably be separate interface
				supplementalData.ChainScore = m_pChainScoreProvider->loadScore();
				supplementalData.State.LastRecalculationHeight = model::ConvertToImportanceHeight(
						chainHeight,
						stateRef.Config.BlockChain.ImportanceGrouping);
				
				stateRef.Score += supplementalData.ChainScore;
				stateRef.State = supplementalData.State;
			}

			void registerCounters(const RegisterCounterFunc& registerCounter) override {
				registerCounter("READERS", [&source = m_packetReaderService]() { return source.numActiveReaders(); });
				registerCounter("WRITERS", [&source = m_packetWriterService]() { return source.numActiveWriters(); });
				registerCounter("TASKS", [&source = m_schedulerService]() { return source.numScheduledTasks(); });
				registerCounter("BLK ELEMENTS", [&source = m_dispatcherService]() { return source.numAddedBlockElements(); });
				registerCounter("TX ELEMENTS", [&source = m_dispatcherService]() { return source.numAddedTransactionElements(); });
			}

			void startServices(thread::MultiServicePool& pool) override {
				// boot services
				m_packetWriterService.boot(pool);
				m_dispatcherService.boot(pool);
				m_packetReaderService.boot(pool);
				m_schedulerService.boot(pool, m_packetWriterService.packetWriters());
			}

			void saveToStorage() override {
				// do nothing
			}

		private:
			std::shared_ptr<ChainScoreProvider> m_pChainScoreProvider;
			std::unique_ptr<mongo::plugins::ExternalCacheStorage> m_pCacheStorage;
			UnconfirmedTransactionsCacheViewProvider m_unconfirmedTransactionsCacheViewProvider;
			std::unique_ptr<cache::UtCache> m_pUnconfirmedTransactionCache;
			LocalNodeApiState m_apiState;

			NetworkPacketWriterService m_packetWriterService;
			DispatcherService m_dispatcherService;
			NetworkPacketReaderService m_packetReaderService;
			SchedulerService m_schedulerService;
		};
	}

	std::unique_ptr<BootedLocalNode> CreateLocalNode(
			const crypto::KeyPair& keyPair,
			config::LocalNodeConfiguration&& config,
			std::unique_ptr<thread::MultiServicePool>&& pMultiServicePool,
			std::unique_ptr<io::BlockStorage>&& pBlockStorage,
			std::shared_ptr<ChainScoreProvider>&& pChainScoreProvider,
			std::unique_ptr<mongo::plugins::ExternalCacheStorage>&& pCacheStorage,
			const UnconfirmedTransactionsCacheViewProvider& unconfirmedTransactionsCacheViewProvider,
			std::unique_ptr<cache::UtCache>&& pUnconfirmedTransactionCache) {
		auto pLocalNode = std::make_unique<DefaultLocalNode>(
				std::move(config),
				std::move(pMultiServicePool),
				std::move(pBlockStorage),
				std::move(pChainScoreProvider),
				std::move(pCacheStorage),
				unconfirmedTransactionsCacheViewProvider,
				std::move(pUnconfirmedTransactionCache),
				keyPair);
		pLocalNode->boot();
		return std::move(pLocalNode);
	}
}}}
