#pragma once
#include "ChainHandlers.h"
#include "DiagnosticHandlers.h"
#include "TransactionHandlers.h"
#include "catapult/model/ChainScore.h"
#include "catapult/utils/DiagnosticCounter.h"

namespace catapult {
	namespace io { class BlockStorageCache; }
	namespace model { class TransactionRegistry; }
}

namespace catapult { namespace handlers {

	/// Configuration for all handlers.
	struct HandlersConfiguration {
		/// Callback for push block handler.
		BlockRangeHandler PushBlockCallback;

		/// Callback for push transactions handler.
		TransactionRangeHandler PushTransactionsCallback;

		/// Chain score supplier.
		handlers::ChainScoreSupplier ChainScoreSupplier;

		/// Configuration for pull blocks handler.
		PullBlocksHandlerConfiguration BlocksHandlerConfig;

		/// The unconfirmed transactions retriever.
		handlers::UnconfirmedTransactionsRetriever UnconfirmedTransactionsRetriever;
	};

	/// Configuration for diagnostic handlers.
	struct DiagnosticHandlersConfiguration {
		/// The diagnostic counters.
		std::vector<utils::DiagnosticCounter> DiagnosticCounters;
	};

	/// Register all handlers inside \a handlers using \a storage, \a registry and \a config.
	void RegisterAllHandlers(
			ionet::ServerPacketHandlers& handlers,
			const io::BlockStorageCache& storage,
			const model::TransactionRegistry& registry,
			const HandlersConfiguration& config);

	/// Register diagnostic handlers inside \a handlers using \a config.
	void RegisterDiagnosticHandlers(ionet::ServerPacketHandlers& handlers, const DiagnosticHandlersConfiguration& config);
}}
