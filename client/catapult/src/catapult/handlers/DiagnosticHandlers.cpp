#include "DiagnosticHandlers.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/ionet/PackedNodeInfo.h"
#include "catapult/model/DiagnosticCounterValue.h"
#include "catapult/utils/DiagnosticCounter.h"

namespace catapult { namespace handlers {

	// region DiagnosticCountersHandler

	namespace {
		auto CreateDiagnosticCountersHandler(const std::vector<utils::DiagnosticCounter>& counters) {
			return [counters](const auto& packet, auto& context) {
				if (!ionet::IsPacketValid(packet, ionet::PacketType::Diagnostic_Counters))
					return;

				auto payloadSize = utils::checked_cast<size_t, uint32_t>(counters.size() * sizeof(model::DiagnosticCounterValue));
				auto pResponsePacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
				pResponsePacket->Type = ionet::PacketType::Diagnostic_Counters;

				auto* pCounterValue = reinterpret_cast<model::DiagnosticCounterValue*>(pResponsePacket->Data());
				for (const auto& counter : counters) {
					pCounterValue->Id = counter.id().value();
					pCounterValue->Value = counter.value();
					++pCounterValue;
				}

				context.response(pResponsePacket);
			};
		}
	}

	void RegisterDiagnosticCountersHandler(ionet::ServerPacketHandlers& handlers, const std::vector<utils::DiagnosticCounter>& counters) {
		handlers.registerHandler(ionet::PacketType::Diagnostic_Counters, CreateDiagnosticCountersHandler(counters));
	}

	// endregion

	// region DiagnosticNodesHandler

	namespace {
		auto CreateDiagnosticNodesHandler(const ionet::NodeContainer& nodeContainer) {
			return [&nodeContainer](const auto& packet, auto& context) {
				if (!ionet::IsPacketValid(packet, ionet::PacketType::Active_Node_Infos))
					return;

				auto view = nodeContainer.view();
				auto activeNodes = ionet::FindAllActiveNodes(view);

				std::vector<std::shared_ptr<ionet::PackedNodeInfo>> packedNodeInfos;
				for (const auto& node : activeNodes) {
					const auto& nodeInfo = view.getNodeInfo(node.identityKey());
					const auto& serviceIds = nodeInfo.services();

					uint32_t nodeInfoSize = sizeof(ionet::PackedNodeInfo);
					nodeInfoSize += static_cast<uint32_t>(serviceIds.size() * sizeof(ionet::PackedConnectionState));
					auto pNodeInfo = utils::MakeSharedWithSize<ionet::PackedNodeInfo>(nodeInfoSize);
					pNodeInfo->Size = nodeInfoSize;
					pNodeInfo->IdentityKey = node.identityKey();
					pNodeInfo->Source = nodeInfo.source();
					pNodeInfo->ConnectionStatesCount = utils::checked_cast<size_t, uint8_t>(serviceIds.size());

					auto* pConnectionState = pNodeInfo->ConnectionStatesPtr();
					for (const auto& serviceId : serviceIds) {
						const auto& connectionState = *nodeInfo.getConnectionState(serviceId);
						pConnectionState->ServiceId = serviceId;
						pConnectionState->Age = connectionState.Age;
						pConnectionState->NumAttempts = connectionState.NumAttempts;
						pConnectionState->NumSuccesses = connectionState.NumSuccesses;
						pConnectionState->NumFailures = connectionState.NumFailures;
						++pConnectionState;
					}

					packedNodeInfos.push_back(pNodeInfo);
				}

				context.response(ionet::PacketPayload::FromEntities(ionet::PacketType::Active_Node_Infos, packedNodeInfos));
			};
		}
	}

	void RegisterDiagnosticNodesHandler(ionet::ServerPacketHandlers& handlers, const ionet::NodeContainer& nodeContainer) {
		handlers.registerHandler(ionet::PacketType::Active_Node_Infos, CreateDiagnosticNodesHandler(nodeContainer));
	}

	// endregion
}}
