#pragma once
#include "LocalNodeApiState.h"
#include "UnconfirmedTransactionsCacheViewProvider.h"
#include "catapult/local/BasicDispatcherService.h"
#include "catapult/local/Sinks.h"

namespace catapult {
	namespace cache { class UtCache; }
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace local { namespace api {

	/// A service for managing api dispatchers.
	class DispatcherService : public BasicDispatcherService {
	public:
		/// Creates a new api dispatcher service around state composed of \a apiState, \a pluginManager and unconfirmed transactions
		/// (read only \a unconfirmedTransactionsCacheViewProvider and write only unconfirmedTransactionsCache)
		/// with sink \a newTransactionsSink.
		DispatcherService(
				const LocalNodeApiState& apiState,
				const plugins::PluginManager& pluginManager,
				const UnconfirmedTransactionsCacheViewProvider& unconfirmedTransactionsCacheViewProvider,
				cache::UtCache& unconfirmedTransactionsCache,
				const SharedNewTransactionsSink& newTransactionsSink);

	public:
		/// Boots the service with \a pool parallelization.
		void boot(thread::MultiServicePool& pool);

	private:
		// state
		const LocalNodeApiState& m_apiState;
		const plugins::PluginManager& m_pluginManager;
		UnconfirmedTransactionsCacheViewProvider m_unconfirmedTransactionsCacheViewProvider;
		cache::UtCache& m_unconfirmedTransactionsCache;
		SharedNewTransactionsSink m_newTransactionsSink;
		std::unique_ptr<chain::UnconfirmedTransactionsUpdater> m_pUnconfirmedTransactionsUpdater;
	};
}}}
