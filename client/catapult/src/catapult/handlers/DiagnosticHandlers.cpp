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

#include "DiagnosticHandlers.h"
#include "BasicProducer.h"
#include "HandlerFactory.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/ionet/PackedNodeInfo.h"
#include "catapult/ionet/PacketPayloadFactory.h"
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

				context.response(ionet::PacketPayload(pResponsePacket));
			};
		}
	}

	void RegisterDiagnosticCountersHandler(ionet::ServerPacketHandlers& handlers, const std::vector<utils::DiagnosticCounter>& counters) {
		handlers.registerHandler(ionet::PacketType::Diagnostic_Counters, CreateDiagnosticCountersHandler(counters));
	}

	// endregion

	// region DiagnosticNodesHandler

	namespace {
		struct DiagnosticNodesTraits {
			static constexpr auto Packet_Type = ionet::PacketType::Active_Node_Infos;

			class Producer : BasicProducer<ionet::NodeSet> {
			public:
				explicit Producer(ionet::NodeContainerView&& view, const ionet::NodeSet& nodes)
						: BasicProducer<ionet::NodeSet>(nodes)
						, m_view(std::move(view))
				{}

				auto operator()() {
					return next([&view = m_view](const auto& node) {
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

						return pNodeInfo;
					});
				}

			private:
				ionet::NodeContainerView m_view;
			};
		};
	}

	void RegisterDiagnosticNodesHandler(ionet::ServerPacketHandlers& handlers, const ionet::NodeContainer& nodeContainer) {
		handlers::BatchHandlerFactory<DiagnosticNodesTraits>::RegisterZero(handlers, [&nodeContainer]() {
			auto view = nodeContainer.view();
			auto pNodes = std::make_unique<ionet::NodeSet>(ionet::FindAllActiveNodes(view)); // used by producer by reference
			auto producer = DiagnosticNodesTraits::Producer(std::move(view), *pNodes);
			return [pNodes = std::move(pNodes), producer = std::move(producer)]() mutable {
				return producer();
			};
		});
	}

	// endregion
}}
