#pragma once
#include "HashPredicateFactory.h"
#include "catapult/local/BasicDispatcherService.h"
#include "catapult/local/LocalNodeStateRef.h"
#include "catapult/local/Sinks.h"

namespace catapult {
	namespace cache { class MemoryUtCache; }
	namespace plugins { class PluginManager; }
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace local { namespace p2p {

	/// A service for managing p2p dispatchers.
	class DispatcherService : public BasicDispatcherService {
	public:
		/// Creates a new p2p dispatcher service around state composed of \a stateRef, \a pluginManager,
		/// \a unconfirmedTransactionsCache and \a hashPredicateFactory.
		/// New blocks and transactions are forwarded to \a newBlockSink and \a newTransactionsSink respectively.
		DispatcherService(
				const LocalNodeStateRef& stateRef,
				const plugins::PluginManager& pluginManager,
				cache::MemoryUtCache& unconfirmedTransactionsCache,
				const HashPredicateFactory& hashPredicateFactory,
				const NewBlockSink& newBlockSink,
				const SharedNewTransactionsSink& newTransactionsSink);

	public:
		/// Boots the service with \a pool parallelization.
		void boot(thread::MultiServicePool& pool);

	private:
		// state
		LocalNodeStateRef m_stateRef;
		const plugins::PluginManager& m_pluginManager;
		cache::MemoryUtCache& m_unconfirmedTransactionsCache;
		HashPredicateFactory m_hashPredicateFactory;
		NewBlockSink m_newBlockSink;
		SharedNewTransactionsSink m_newTransactionsSink;
		std::unique_ptr<chain::UnconfirmedTransactionsUpdater> m_pUnconfirmedTransactionsUpdater;
	};
}}}
