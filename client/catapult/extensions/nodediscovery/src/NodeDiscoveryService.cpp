#include "NodeDiscoveryService.h"
#include "BatchPeersRequestor.h"
#include "NodePingRequestor.h"
#include "PeersProcessor.h"
#include "nodediscovery/src/handlers/NodeDiscoveryHandlers.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/extensions/NetworkUtils.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/ionet/NetworkNode.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/subscribers/NodeSubscriber.h"

namespace catapult { namespace nodediscovery {

	namespace {
		constexpr auto Service_Name = "nd.ping_requestor";
		using ConstNetworkNodePointer = std::shared_ptr<const ionet::NetworkNode>;

		thread::Task CreatePingTask(
				const extensions::PacketPayloadSink& packetPayloadSink,
				const ConstNetworkNodePointer& pLocalNetworkNode) {
			return thread::CreateNamedTask("node discovery ping task", [packetPayloadSink, pLocalNetworkNode]() {
				packetPayloadSink(ionet::PacketPayload::FromEntity(ionet::PacketType::Node_Discovery_Push_Ping, pLocalNetworkNode));
				return thread::make_ready_future(thread::TaskResult::Continue);
			});
		}

		thread::Task CreatePeersTask(
				const utils::TimeSpan& timeout,
				net::PacketIoPickerContainer& packetIoPickers,
				const handlers::NodesConsumer& nodesConsumer) {
			BatchPeersRequestor requestor(packetIoPickers, nodesConsumer);
			return thread::CreateNamedTask("node discovery peers task", [timeout, requestor]() {
				return requestor.findPeersOfPeers(timeout).then([](auto&&) {
					return thread::TaskResult::Continue;
				});
			});
		}

		handlers::NodeConsumer CreatePushNodeConsumer(extensions::ServiceState& state) {
			return [&nodeContainer = state.nodes(), &nodeSubscriber = state.nodeSubscriber()](const auto& node) {
				nodeSubscriber.notifyNode(node);
				nodeContainer.modifier().add(node, ionet::NodeSource::Dynamic);
			};
		}

		class NodeDiscoveryServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			explicit NodeDiscoveryServiceRegistrar(const ConstNetworkNodePointer& pLocalNetworkNode)
					: m_pLocalNetworkNode(pLocalNetworkNode)
			{}

		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "NodeDiscovery", extensions::ServiceRegistrarPhase::Post_Packet_Io_Pickers };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				locator.registerServiceCounter<NodePingRequestor>(Service_Name, "ACTIVE PINGS", [](const auto& requestor) {
					return requestor.numActiveConnections();
				});
				locator.registerServiceCounter<NodePingRequestor>(Service_Name, "TOTAL PINGS", [](const auto& requestor) {
					return requestor.numTotalPingRequests();
				});
				locator.registerServiceCounter<NodePingRequestor>(Service_Name, "SUCCESS PINGS", [](const auto& requestor) {
					return requestor.numSuccessfulPingRequests();
				});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				auto networkIdentifier = state.config().BlockChain.Network.Identifier;

				// create callbacks
				auto pushNodeConsumer = CreatePushNodeConsumer(state);

				// register services
				auto connectionSettings = extensions::GetConnectionSettings(state.config());
				auto pServiceGroup = state.pool().pushServiceGroup("node_discovery");
				auto pNodePingRequestor = pServiceGroup->pushService(
						CreateNodePingRequestor,
						locator.keyPair(),
						connectionSettings,
						networkIdentifier);

				locator.registerService(Service_Name, pNodePingRequestor);

				// set handlers
				auto& nodeContainer = state.nodes();
				auto& pingRequestor = *pNodePingRequestor;
				handlers::RegisterNodeDiscoveryPushPingHandler(state.packetHandlers(), networkIdentifier, pushNodeConsumer);
				handlers::RegisterNodeDiscoveryPullPingHandler(state.packetHandlers(), *m_pLocalNetworkNode);

				PeersProcessor peersProcessor(nodeContainer, pingRequestor, networkIdentifier, pushNodeConsumer);
				auto pushPeersHandler = [peersProcessor](const auto& candidateNodes) {
					peersProcessor.process(candidateNodes);
				};
				handlers::RegisterNodeDiscoveryPushPeersHandler(state.packetHandlers(), pushPeersHandler);
				handlers::RegisterNodeDiscoveryPullPeersHandler(state.packetHandlers(), [&nodeContainer]() {
					return ionet::FindAllActiveNodes(nodeContainer.view());
				});

				// add task
				state.tasks().push_back(CreatePingTask(state.hooks().packetPayloadSink(), m_pLocalNetworkNode));
				state.tasks().push_back(CreatePeersTask(state.config().Node.SyncTimeout, state.packetIoPickers(), pushPeersHandler));
			}

		private:
			ConstNetworkNodePointer m_pLocalNetworkNode;
		};
	}

	DECLARE_SERVICE_REGISTRAR(NodeDiscovery)(const ConstNetworkNodePointer& pLocalNetworkNode) {
		return std::make_unique<NodeDiscoveryServiceRegistrar>(pLocalNetworkNode);
	}
}}
