#include "PtService.h"
#include "PtBootstrapperService.h"
#include "partialtransaction/src/api/RemotePtApi.h"
#include "partialtransaction/src/chain/PtSynchronizer.h"
#include "catapult/cache/MemoryPtCache.h"
#include "catapult/extensions/NetworkUtils.h"
#include "catapult/extensions/PeersConnectionTasks.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/SynchronizerTaskCallbacks.h"
#include "catapult/net/PacketWriters.h"
#include "catapult/thread/MultiServicePool.h"

namespace catapult { namespace partialtransaction {

	namespace {
		constexpr auto Service_Name = "api.partial";

		thread::Task CreateConnectPeersTask(extensions::ServiceState& state, net::PacketWriters& packetWriters) {
			const auto& connectionsConfig = state.config().Node.OutgoingConnections;
			auto& nodes = state.nodes();

			auto serviceId = ionet::ServiceIdentifier(0x50415254);
			auto selector = extensions::CreateNodeSelector(serviceId, ionet::NodeRoles::Api, connectionsConfig, nodes);
			auto task = extensions::CreateConnectPeersTask(nodes, packetWriters, serviceId, selector);
			task.Name += " for service Pt";
			return task;
		}

		thread::Task CreatePullPtTask(
				extensions::ServiceLocator& locator,
				const extensions::ServiceState& state,
				net::PacketWriters& packetWriters) {
			const auto& ptCache = GetMemoryPtCache(locator);
			const auto& serverHooks = GetPtServerHooks(locator);
			auto ptSynchronizer = chain::CreatePtSynchronizer(
					[&ptCache]() { return ptCache.view().shortHashPairs(); },
					serverHooks.cosignedTransactionInfosConsumer());

			thread::Task task;
			task.Name = "pull partial transactions task";
			task.Callback = CreateChainSyncAwareSynchronizerTaskCallback(
					std::move(ptSynchronizer),
					api::CreateRemotePtApi,
					packetWriters,
					state,
					task.Name);
			return task;
		}

		class PtServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "Pt", extensions::ServiceRegistrarPhase::Post_Extended_Range_Consumers };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				locator.registerServiceCounter<net::PacketWriters>(Service_Name, "PT WRITERS", [](const auto& writers) {
					return writers.numActiveWriters();
				});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				auto connectionSettings = extensions::GetConnectionSettings(state.config());
				auto pServiceGroup = state.pool().pushServiceGroup("partial");
				auto pWriters = pServiceGroup->pushService(net::CreatePacketWriters, locator.keyPair(), connectionSettings);

				locator.registerService(Service_Name, pWriters);
				state.packetIoPickers().insert(*pWriters, ionet::NodeRoles::Api);

				// add tasks
				state.tasks().push_back(CreateConnectPeersTask(state, *pWriters));
				state.tasks().push_back(CreatePullPtTask(locator, state, *pWriters));
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(Pt)() {
		return std::make_unique<PtServiceRegistrar>();
	}
}}
