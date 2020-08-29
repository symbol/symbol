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

#include "SyncService.h"
#include "NetworkPacketWritersService.h"
#include "catapult/api/LocalChainApi.h"
#include "catapult/api/RemoteChainApi.h"
#include "catapult/api/RemoteTransactionApi.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/chain/UtSynchronizer.h"
#include "catapult/config/CatapultConfiguration.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/extensions/PeersConnectionTasks.h"
#include "catapult/extensions/SynchronizerTaskCallbacks.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace sync {

	namespace {
		constexpr auto Sync_Source = disruptor::InputSource::Remote_Pull;
		constexpr auto Service_Id = ionet::ServiceIdentifier(0x53594E43);

		thread::Task CreateConnectPeersTask(extensions::ServiceState& state, net::PacketWriters& packetWriters) {
			auto settings = extensions::SelectorSettings(
					state.cache(),
					state.config().BlockChain.TotalChainImportance,
					state.nodes(),
					Service_Id,
					ionet::NodeRoles::Peer,
					state.config().Node.OutgoingConnections);
			auto task = extensions::CreateConnectPeersTask(settings, packetWriters);
			task.Name += " for service Sync";
			return task;
		}

		chain::ChainSynchronizerConfiguration CreateChainSynchronizerConfiguration(const config::CatapultConfiguration& config) {
			chain::ChainSynchronizerConfiguration chainSynchronizerConfig;
			chainSynchronizerConfig.MaxHashesPerSyncAttempt = config.Node.MaxHashesPerSyncAttempt;
			chainSynchronizerConfig.MaxBlocksPerSyncAttempt = config.Node.MaxBlocksPerSyncAttempt;
			chainSynchronizerConfig.MaxChainBytesPerSyncAttempt = config.Node.MaxChainBytesPerSyncAttempt.bytes32();
			chainSynchronizerConfig.MaxRollbackBlocks = config.BlockChain.MaxRollbackBlocks;
			return chainSynchronizerConfig;
		}

		thread::Task CreateSynchronizerTask(const extensions::ServiceState& state, net::PacketWriters& packetWriters) {
			const auto& config = state.config();
			auto chainSynchronizer = chain::CreateChainSynchronizer(
					api::CreateLocalChainApi(
							state.storage(),
							[&score = state.score()]() { return score.get(); },
							state.hooks().localFinalizedHeightSupplier()),
					CreateChainSynchronizerConfiguration(config),
					extensions::CreateLocalFinalizedHeightSupplier(state),
					state.hooks().completionAwareBlockRangeConsumerFactory()(Sync_Source));

			thread::Task task;
			task.Name = "synchronizer task";
			task.Callback = CreateSynchronizerTaskCallback(
					std::move(chainSynchronizer),
					api::CreateRemoteChainApi,
					packetWriters,
					state,
					task.Name);
			return task;
		}

		thread::Task CreatePullUtTask(const extensions::ServiceState& state, net::PacketWriters& packetWriters) {
			auto utSynchronizer = chain::CreateUtSynchronizer(
					state.config().Node.MinFeeMultiplier,
					[&cache = state.utCache()]() { return cache.view().shortHashes(); },
					state.hooks().transactionRangeConsumerFactory()(Sync_Source));

			thread::Task task;
			task.Name = "pull unconfirmed transactions task";
			task.Callback = CreateChainSyncAwareSynchronizerTaskCallback(
					std::move(utSynchronizer),
					api::CreateRemoteTransactionApi,
					packetWriters,
					state,
					task.Name);
			return task;
		}

		class SyncServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "Sync", extensions::ServiceRegistrarPhase::Post_Range_Consumers };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no additional counters
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				auto& packetWriters = *GetPacketWriters(locator);

				// add tasks
				state.tasks().push_back(CreateConnectPeersTask(state, packetWriters));
				state.tasks().push_back(CreateSynchronizerTask(state, packetWriters));
				state.tasks().push_back(CreatePullUtTask(state, packetWriters));
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(Sync)() {
		return std::make_unique<SyncServiceRegistrar>();
	}
}}
