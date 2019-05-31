/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "SyncSourceService.h"
#include "catapult/cache_tx/MemoryUtCache.h"
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
			config.UtRetriever = [&cache = state.utCache()](auto minFeeMultiplier, const auto& shortHashes) {
				return cache.view().unknownTransactions(minFeeMultiplier, shortHashes);
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
