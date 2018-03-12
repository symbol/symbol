#include "NetworkPacketReadersService.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/extensions/NetworkUtils.h"
#include "catapult/extensions/PeersConnectionTasks.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/thread/MultiServicePool.h"

namespace catapult { namespace packetserver {

	namespace {
		constexpr auto Service_Name = "readers";

		thread::Task CreateAgePeersTask(extensions::ServiceState& state, net::ConnectionContainer& connectionContainer) {
			const auto& connectionsConfig = state.config().Node.IncomingConnections;
			auto& nodes = state.nodes();

			auto serviceId = ionet::ServiceIdentifier(0x52454144);
			auto selector = extensions::CreateRemoveOnlyNodeSelector(serviceId, connectionsConfig, nodes);
			auto task = extensions::CreateAgePeersTask(nodes, connectionContainer, serviceId, selector);
			task.Name += " for service Readers";
			return task;
		}

		class NetworkPacketReadersServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "NetworkPacketReaders", extensions::ServiceRegistrarPhase::Post_Packet_Handlers };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				locator.registerServiceCounter<net::PacketReaders>(Service_Name, "READERS", [](const auto& writers) {
					return writers.numActiveReaders();
				});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				const auto& config = state.config();
				auto pServiceGroup = state.pool().pushServiceGroup(Service_Name);
				auto pReaders = pServiceGroup->pushService(
						net::CreatePacketReaders,
						state.packetHandlers(),
						locator.keyPair(),
						extensions::GetConnectionSettings(config),
						extensions::GetMaxIncomingConnectionsPerIdentity(config.Node.Local.Roles));
				extensions::BootServer(*pServiceGroup, config.Node.Port, config, [&acceptor = *pReaders](
						const auto& socketInfo,
						const auto& callback) {
					acceptor.accept(socketInfo, callback);
				});

				locator.registerService(Service_Name, pReaders);

				// add tasks
				state.tasks().push_back(CreateAgePeersTask(state, *pReaders));
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(NetworkPacketReaders)() {
		return std::make_unique<NetworkPacketReadersServiceRegistrar>();
	}
}}
