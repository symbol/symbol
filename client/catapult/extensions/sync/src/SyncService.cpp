#include "SyncService.h"
#include "NetworkPacketWritersService.h"
#include "catapult/api/LocalChainApi.h"
#include "catapult/api/RemoteChainApi.h"
#include "catapult/api/RemoteTransactionApi.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/chain/UtSynchronizer.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/extensions/PeersConnectionTasks.h"
#include "catapult/extensions/SynchronizerTaskCallbacks.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace sync {

	namespace {
		constexpr auto Sync_Source = disruptor::InputSource::Remote_Pull;

		thread::Task CreateConnectPeersTask(extensions::ServiceState& state, net::PacketWriters& packetWriters) {
			const auto& connectionsConfig = state.config().Node.OutgoingConnections;
			auto& nodes = state.nodes();

			auto serviceId = ionet::ServiceIdentifier(0x53594E43);
			auto selector = extensions::CreateNodeSelector(serviceId, ionet::NodeRoles::Peer, connectionsConfig, nodes);
			auto task = extensions::CreateConnectPeersTask(nodes, packetWriters, serviceId, selector);
			task.Name += " for service Sync";
			return task;
		}

		chain::ChainSynchronizerConfiguration CreateChainSynchronizerConfiguration(const config::LocalNodeConfiguration& config) {
			chain::ChainSynchronizerConfiguration chainSynchronizerConfig;
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
							config.Node.MaxBlocksPerSyncAttempt),
					CreateChainSynchronizerConfiguration(config),
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
