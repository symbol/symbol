#include "RegisterHandlers.h"
#include "catapult/model/BlockChainConfiguration.h"

namespace catapult { namespace handlers {

	void RegisterAllHandlers(
			ionet::ServerPacketHandlers& handlers,
			const io::BlockStorageCache& storage,
			const model::TransactionRegistry& registry,
			const HandlersConfiguration& config) {
		RegisterPushBlockHandler(handlers, registry, config.PushBlockCallback);
		RegisterPullBlockHandler(handlers, storage);

		RegisterChainInfoHandler(handlers, storage, config.ChainScoreSupplier);
		RegisterBlockHashesHandler(
				handlers,
				storage,
				static_cast<uint32_t>(config.BlocksHandlerConfig.MaxBlocks));
		RegisterPullBlocksHandler(handlers, storage, config.BlocksHandlerConfig);

		RegisterPushTransactionsHandler(handlers, registry, config.PushTransactionsCallback);
		RegisterPullTransactionsHandler(handlers, config.UnconfirmedTransactionsRetriever);
	}

	void RegisterDiagnosticHandlers(ionet::ServerPacketHandlers& handlers, const DiagnosticHandlersConfiguration& config) {
		RegisterDiagnosticCountersHandler(handlers, config.DiagnosticCounters);
	}
}}
