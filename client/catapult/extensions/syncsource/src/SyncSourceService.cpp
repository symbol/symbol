#include "SyncSourceService.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/extensions/ServerHooksUtils.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/handlers/ChainHandlers.h"
#include "catapult/handlers/TransactionHandlers.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace syncsource {

	namespace {
		void SetConfig(handlers::PullBlocksHandlerConfiguration& blocksHandlerConfig, const config::NodeConfiguration& nodeConfig) {
			blocksHandlerConfig.MaxBlocks = nodeConfig.MaxBlocksPerSyncAttempt;
			blocksHandlerConfig.MaxResponseBytes = nodeConfig.MaxChainBytesPerSyncAttempt.bytes32();
		}

		struct HandlersConfiguration {
			handlers::BlockRangeHandler PushBlockCallback;
			model::ChainScoreSupplier ChainScoreSupplier;
			handlers::PullBlocksHandlerConfiguration BlocksHandlerConfig;
			handlers::UtRetriever UtRetriever;
		};

		HandlersConfiguration CreateHandlersConfiguration(const extensions::ServiceState& state) {
			HandlersConfiguration config;
			config.PushBlockCallback = extensions::CreateBlockPushEntityCallback(state.hooks());

			config.ChainScoreSupplier = [&chainScore = state.score()]() { return chainScore.get(); };
			config.UtRetriever = [&cache = state.utCache()](const auto& shortHashes) {
				return cache.view().unknownTransactions(shortHashes);
			};

			SetConfig(config.BlocksHandlerConfig, state.config().Node);
			return config;
		}

		void RegisterAllHandlers(
				ionet::ServerPacketHandlers& handlers,
				const io::BlockStorageCache& storage,
				const model::TransactionRegistry& registry,
				const HandlersConfiguration& config) {
			handlers::RegisterPushBlockHandler(handlers, registry, config.PushBlockCallback);
			handlers::RegisterPullBlockHandler(handlers, storage);

			handlers::RegisterChainInfoHandler(handlers, storage, config.ChainScoreSupplier);
			handlers::RegisterBlockHashesHandler(handlers, storage, static_cast<uint32_t>(config.BlocksHandlerConfig.MaxBlocks));
			handlers::RegisterPullBlocksHandler(handlers, storage, config.BlocksHandlerConfig);

			handlers::RegisterPullTransactionsHandler(handlers, config.UtRetriever);
		}

		class SyncSourceServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "SyncSource", extensions::ServiceRegistrarPhase::Post_Range_Consumers };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no additional counters
			}

			void registerServices(extensions::ServiceLocator&, extensions::ServiceState& state) override {
				// add handlers
				RegisterAllHandlers(
						state.packetHandlers(),
						state.storage(),
						state.pluginManager().transactionRegistry(),
						CreateHandlersConfiguration(state));
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(SyncSource)() {
		return std::make_unique<SyncSourceServiceRegistrar>();
	}
}}
