#include "NetworkPacketReaderService.h"
#include "LocalNodeFunctionalPlugins.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/local/LocalNodeChainScore.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace local { namespace p2p {

	NetworkPacketReaderService::NetworkPacketReaderService(
			const crypto::KeyPair& keyPair,
			const LocalNodeStateConstRef& stateRef,
			const plugins::PluginManager& pluginManager,
			const cache::MemoryUtCache& unconfirmedTransactionsCache,
			const NetworkChainHeight& networkChainHeight,
			const handlers::BlockRangeHandler& blockRangeConsumer,
			const handlers::TransactionRangeHandler& transactionRangeConsumer)
			: BasicNetworkPacketReaderService(keyPair, stateRef)
			, m_pluginManager(pluginManager)
			, m_unconfirmedTransactionsCache(unconfirmedTransactionsCache)
			, m_networkChainHeight(networkChainHeight)
			, m_blockRangeConsumer(blockRangeConsumer)
			, m_transactionRangeConsumer(transactionRangeConsumer)
	{}

	namespace {
		handlers::DiagnosticHandlersConfiguration CreateDiagnosticHandlersConfiguration(
				const plugins::PluginManager& pluginManager,
				const cache::CatapultCache& cache) {
			handlers::DiagnosticHandlersConfiguration config;
			pluginManager.addDiagnosticCounters(config.DiagnosticCounters, cache);
			return config;
		}
	}

	template<typename TConsumer>
	auto NetworkPacketReaderService::createPushEntityCallback(const TConsumer& consumer, const io::BlockStorageCache& storage) {
		const auto& networkChainHeight = m_networkChainHeight;
		auto isChainSynced = CreateChainSyncedPredicate(storage, [&networkChainHeight]() {
			return networkChainHeight.load();
		});

		return [isChainSynced, consumer](auto&& range) {
			if (isChainSynced())
				consumer(std::move(range));
		};
	}

	namespace {
		void SetConfig(handlers::PullBlocksHandlerConfiguration& blocksHandlerConfig, const config::NodeConfiguration& nodeConfig) {
			blocksHandlerConfig.MinBlocks = nodeConfig.MinBlocksPerSyncAttempt;
			blocksHandlerConfig.MaxBlocks = nodeConfig.MaxBlocksPerSyncAttempt;
			blocksHandlerConfig.MinResponseBytes = nodeConfig.MinChainBytesPerSyncAttempt.bytes32();
			blocksHandlerConfig.MaxResponseBytes = nodeConfig.MaxChainBytesPerSyncAttempt.bytes32();
		}
	}

	handlers::HandlersConfiguration NetworkPacketReaderService::createHandlersConfiguration(const LocalNodeStateConstRef& stateRef) {
		handlers::HandlersConfiguration config;
		config.PushBlockCallback = createPushEntityCallback(m_blockRangeConsumer, stateRef.Storage);
		config.PushTransactionsCallback = createPushEntityCallback(m_transactionRangeConsumer, stateRef.Storage);

		const auto& chainScore = stateRef.Score;
		config.ChainScoreSupplier = [&chainScore]() { return chainScore.get(); };

		const auto& unconfirmedTransactionsCache = m_unconfirmedTransactionsCache;
		config.UnconfirmedTransactionsRetriever = [&unconfirmedTransactionsCache](const auto& shortHashes) {
			return unconfirmedTransactionsCache.view().unknownTransactions(shortHashes);
		};

		SetConfig(config.BlocksHandlerConfig, stateRef.Config.Node);
		return config;
	}

	void NetworkPacketReaderService::registerHandlers(
			ionet::ServerPacketHandlers& serverPacketHandlers,
			const LocalNodeStateConstRef& stateRef) {
		handlers::RegisterAllHandlers(
				serverPacketHandlers,
				stateRef.Storage,
				m_pluginManager.transactionRegistry(),
				createHandlersConfiguration(stateRef));

		// register diagnostic handlers
		handlers::RegisterDiagnosticHandlers(
				serverPacketHandlers,
				CreateDiagnosticHandlersConfiguration(m_pluginManager, stateRef.Cache));
		m_pluginManager.addDiagnosticHandlers(serverPacketHandlers, stateRef.Cache);
	}
}}}
