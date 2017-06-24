#include "LocalNode.h"
#include "DispatcherService.h"
#include "HashPredicateFactory.h"
#include "LocalNodeStateStorage.h"
#include "MultiBlockLoader.h"
#include "NetworkPacketReaderService.h"
#include "NetworkPacketWriterService.h"
#include "SchedulerService.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/cache/SupplementalData.h"
#include "catapult/chain/UnlockedAccounts.h"
#include "catapult/io/FileBasedStorage.h"
#include "catapult/local/BasicLocalNode.h"
#include "catapult/local/ConfigurationUtils.h"
#include "catapult/local/NemesisBlockLoader.h"
#include "catapult/local/PluginUtils.h"

namespace catapult { namespace local { namespace p2p {

	namespace {
		constexpr auto Remote_Push_Source = disruptor::InputSource::Remote_Push;
		constexpr auto Remote_Pull_Source = disruptor::InputSource::Remote_Pull;

		std::unique_ptr<chain::UnlockedAccounts> CreateUnlockedAccounts(const config::LocalNodeConfiguration& config) {
			auto pUnlockedAccounts = std::make_unique<chain::UnlockedAccounts>(config.User.MaxUnlockedAccounts);
			if (config.User.IsAutoHarvestingEnabled) {
				// unlock boot account if it is eligible to harvest the next block
				auto unlockResult = pUnlockedAccounts->modifier().add(crypto::KeyPair::FromString(config.User.HarvestKey));
				CATAPULT_LOG(info) << "Unlocked boot key for harvesting with result " << unlockResult;
			}

			return pUnlockedAccounts;
		}

		class DefaultLocalNode final : public BasicLocalNode {
		public:
			DefaultLocalNode(
					config::LocalNodeConfiguration&& config,
					std::unique_ptr<thread::MultiServicePool>&& pMultiServicePool,
					std::unique_ptr<io::BlockStorage>&& pBlockStorage,
					const crypto::KeyPair& keyPair)
					: BasicLocalNode(std::move(config), std::move(pMultiServicePool), std::move(pBlockStorage))
					, m_unconfirmedTransactionsCache(GetUnconfirmedTransactionsCacheOptions(stateCref().Config.Node))
					, m_networkChainHeight(0)
					, m_packetWriterService(keyPair, stateCref().Config)
					, m_dispatcherService(
							stateRef(),
							pluginManager(),
							m_unconfirmedTransactionsCache,
							HashPredicateFactory(m_unconfirmedTransactionsCache, stateRef().Cache),
							m_packetWriterService.createNewBlockSink(),
							m_packetWriterService.createNewTransactionsSink())
					, m_packetReaderService(
							keyPair,
							stateCref(),
							pluginManager(),
							m_unconfirmedTransactionsCache,
							m_networkChainHeight,
							m_dispatcherService.createBlockRangeConsumerFactory()(Remote_Push_Source),
							m_dispatcherService.createTransactionRangeConsumerFactory()(Remote_Push_Source))
					, m_schedulerService(
							stateCref(),
							pluginManager(),
							m_unconfirmedTransactionsCache,
							m_networkChainHeight,
							m_dispatcherService.createCompletionAwareBlockRangeConsumerFactory(),
							m_dispatcherService.createTransactionRangeConsumerFactory()(Remote_Pull_Source))
			{}

			~DefaultLocalNode() {
				shutdown();
			}

		private:
			void registerPlugins(const RegisterPluginFunc& registerPlugin) override {
				registerPlugin("catapult.plugins.hashcache");
			}

			void loadFromStorage() override {
				auto stateRef = this->stateRef();
				loadBlockChain(stateRef);
				m_pUnlockedAccounts = CreateUnlockedAccounts(stateRef.Config);
			}

			void registerCounters(const RegisterCounterFunc& registerCounter) override {
				registerCounter("READERS", [&source = m_packetReaderService]() { return source.numActiveReaders(); });
				registerCounter("WRITERS", [&source = m_packetWriterService]() { return source.numActiveWriters(); });
				registerCounter("TASKS", [&source = m_schedulerService]() { return source.numScheduledTasks(); });
				registerCounter("BLK ELEMENTS", [&source = m_dispatcherService]() { return source.numAddedBlockElements(); });
				registerCounter("TX ELEMENTS", [&source = m_dispatcherService]() { return source.numAddedTransactionElements(); });

				registerCounter("B WRITERS", [&source = m_packetWriterService]() { return source.numActiveBroadcastWriters(); });
				registerCounter("UNLKED ACCTS", [&source = *m_pUnlockedAccounts]() { return source.view().size(); });

				registerCounter("UNCNFRMTX C", [&source = m_unconfirmedTransactionsCache]() { return source.view().size(); });
			}

			void startServices(thread::MultiServicePool& pool) override {
				// boot services
				m_packetWriterService.boot(pool);
				m_dispatcherService.boot(pool);
				m_packetReaderService.boot(pool);
				m_schedulerService.boot(pool, m_packetWriterService.packetWriters(), *m_pUnlockedAccounts);
			}

			void saveToStorage() override {
				auto stateRef = this->stateCref();
				SaveState(
						stateRef.Config.User.DataDirectory,
						stateRef.Cache,
						cache::SupplementalData{ stateRef.State, stateRef.Score.get() });
			}

		private:
			void loadBlockChainFromStorage(const LocalNodeStateRef& stateRef) {
				const auto& pluginManager = this->pluginManager();
				auto pObserver = CreateEntityObserver(pluginManager);
				auto pPermanentObserver = CreatePermanentEntityObserver(pluginManager);

				auto storageView = stateRef.Storage.view();
				auto pLastBlock = storageView.loadBlock(storageView.chainHeight());
				auto observerFactory = CreateBlockDependentEntityObserverFactory(
						*pLastBlock,
						stateRef.Config.BlockChain,
						*pObserver,
						*pPermanentObserver);

				auto pNemesisBlock = storageView.loadBlock(Height(1));
				LoadNemesisBlock(pluginManager.transactionRegistry(), observerFactory(*pNemesisBlock), stateRef);

				auto score = LoadBlockChain(observerFactory, stateRef);
				CATAPULT_LOG(info) << "loaded a block chain with score " << score;
				stateRef.Score += score;
			}

			void loadBlockChain(const LocalNodeStateRef& stateRef) {
				cache::SupplementalData supplementalData;
				bool isStateLoaded = false;
				try {
					isStateLoaded = LoadState(stateRef.Config.User.DataDirectory, stateRef.Cache, supplementalData);
				} catch (...) {
					CATAPULT_LOG(error) << "error when loading state, remove state directory and start again";
					throw;
				}

				if (isStateLoaded) {
					stateRef.Score += supplementalData.ChainScore;
					stateRef.State = supplementalData.State;

					CATAPULT_LOG(info) << "Loaded a block chain from cache, score " << stateRef.Score.get();
				} else {
					CATAPULT_LOG(debug) << "loading the block chain";
					loadBlockChainFromStorage(stateRef);
				}
			}

		private:
			cache::MemoryUtCache m_unconfirmedTransactionsCache;
			std::unique_ptr<chain::UnlockedAccounts> m_pUnlockedAccounts;
			NetworkChainHeight m_networkChainHeight;

			NetworkPacketWriterService m_packetWriterService;
			DispatcherService m_dispatcherService;
			NetworkPacketReaderService m_packetReaderService;
			SchedulerService m_schedulerService;
		};
	}

	std::unique_ptr<BootedLocalNode> CreateLocalNode(
			const crypto::KeyPair& keyPair,
			config::LocalNodeConfiguration&& config,
			std::unique_ptr<thread::MultiServicePool>&& pMultiServicePool) {
		auto pBlockStorage = std::make_unique<io::FileBasedStorage>(config.User.DataDirectory);
		auto pLocalNode = std::make_unique<DefaultLocalNode>(
				std::move(config),
				std::move(pMultiServicePool),
				std::move(pBlockStorage),
				keyPair);
		pLocalNode->boot();
		return std::move(pLocalNode);
	}
}}}
