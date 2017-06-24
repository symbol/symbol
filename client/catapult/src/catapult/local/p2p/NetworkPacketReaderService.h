#pragma once
#include "NetworkHeight.h"
#include "catapult/handlers/RegisterHandlers.h"
#include "catapult/local/BasicNetworkPacketReaderService.h"

namespace catapult {
	namespace cache { class MemoryUtCache; }
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace local { namespace p2p {

	/// A service for handling p2p client requests (used for reading packets from tcp).
	class NetworkPacketReaderService : public BasicNetworkPacketReaderService {
	public:
		/// Creates a service around \a keyPair, \a stateRef, \a pluginManager, \a unconfirmedTransactionsCache
		/// and \a networkChainHeight.
		/// Pushed entities are forwarded to \a blockRangeConsumer and \a transactionRangeConsumer.
		NetworkPacketReaderService(
				const crypto::KeyPair& keyPair,
				const LocalNodeStateConstRef& stateRef,
				const plugins::PluginManager& pluginManager,
				const cache::MemoryUtCache& unconfirmedTransactionsCache,
				const NetworkChainHeight& networkChainHeight,
				const handlers::BlockRangeHandler& blockRangeConsumer,
				const handlers::TransactionRangeHandler& transactionRangeConsumer);

	private:
		template<typename TConsumer>
		auto createPushEntityCallback(const TConsumer& consumer, const io::BlockStorageCache& storage);

		handlers::HandlersConfiguration createHandlersConfiguration(const LocalNodeStateConstRef& stateRef);

	private:
		void registerHandlers(ionet::ServerPacketHandlers& serverPacketHandlers, const LocalNodeStateConstRef& stateRef) override;

	private:
		// state
		const plugins::PluginManager& m_pluginManager;
		const cache::MemoryUtCache& m_unconfirmedTransactionsCache;
		const NetworkChainHeight& m_networkChainHeight;
		handlers::BlockRangeHandler m_blockRangeConsumer;
		handlers::TransactionRangeHandler m_transactionRangeConsumer;
	};
}}}
