#include "ApiNetworkPacketWritersService.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/extensions/NetworkUtils.h"
#include "catapult/extensions/PeersConnectionTasks.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/extensions/ServiceUtils.h"
#include "catapult/ionet/BroadcastUtils.h"
#include "catapult/thread/MultiServicePool.h"

namespace catapult { namespace eventsource {

	namespace {
		constexpr auto Service_Name = "api.writers";
		using BlockSink = extensions::NewBlockSink;
		using TransactionsSink = extensions::SharedNewTransactionsSink;

		thread::Task CreateAgePeersTask(extensions::ServiceState& state, net::ConnectionContainer& connectionContainer) {
			const auto& connectionsConfig = state.config().Node.IncomingConnections;
			auto& nodes = state.nodes();

			auto serviceId = ionet::ServiceIdentifier(0x41504957);
			auto selector = extensions::CreateRemoveOnlyNodeSelector(serviceId, connectionsConfig, nodes);
			auto task = extensions::CreateAgePeersTask(nodes, connectionContainer, serviceId, selector);
			task.Name += " for service Api Writers";
			return task;
		}

		class ApiNetworkPacketWritersServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "ApiNetworkPacketWriters", extensions::ServiceRegistrarPhase::Initial };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				locator.registerServiceCounter<net::PacketWriters>(Service_Name, "B WRITERS", [](const auto& writers) {
					return writers.numActiveWriters();
				});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				const auto& config = state.config();
				auto connectionSettings = extensions::GetConnectionSettings(config);
				auto pServiceGroup = state.pool().pushServiceGroup("api");
				auto pWriters = pServiceGroup->pushService(net::CreatePacketWriters, locator.keyPair(), connectionSettings);
				extensions::BootServer(*pServiceGroup, config.Node.ApiPort, config, [&acceptor = *pWriters](
						const auto& socketInfo,
						const auto& callback) {
					acceptor.accept(socketInfo.socket(), callback);
				});

				locator.registerService(Service_Name, pWriters);

				// add sinks
				state.hooks().addNewBlockSink(extensions::CreatePushEntitySink<BlockSink>(locator, Service_Name));
				state.hooks().addNewTransactionsSink(extensions::CreatePushEntitySink<TransactionsSink>(locator, Service_Name));

				// add tasks
				state.tasks().push_back(CreateAgePeersTask(state, *pWriters));
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(ApiNetworkPacketWriters)() {
		return std::make_unique<ApiNetworkPacketWritersServiceRegistrar>();
	}
}}
