#include "PeersProcessor.h"
#include "NodePingRequestor.h"
#include "NodePingUtils.h"
#include "catapult/ionet/NodeContainer.h"

namespace catapult { namespace nodediscovery {

	PeersProcessor::PeersProcessor(
			const ionet::NodeContainer& nodeContainer,
			NodePingRequestor& pingRequestor,
			model::NetworkIdentifier networkIdentifier,
			const NodeConsumer& newPartnerNodeConsumer)
			: m_nodeContainer(nodeContainer)
			, m_pingRequestor(pingRequestor)
			, m_networkIdentifier(networkIdentifier)
			, m_newPartnerNodeConsumer(newPartnerNodeConsumer)
	{}

	void PeersProcessor::process(const ionet::NodeSet& candidateNodes) const {
		for (const auto& candidateNode : SelectUnknownNodes(m_nodeContainer.view(), candidateNodes)) {
			CATAPULT_LOG(debug) << "initiating ping with " << candidateNode;
			process(candidateNode);
		}
	}

	void PeersProcessor::process(const ionet::Node& candidateNode) const {
		auto networkIdentifier = m_networkIdentifier;
		auto newPartnerNodeConsumer = m_newPartnerNodeConsumer;
		m_pingRequestor.requestPing(candidateNode, [candidateNode, networkIdentifier, newPartnerNodeConsumer](
				auto result,
				const auto& responseNode) {
			CATAPULT_LOG(info) << "ping with " << candidateNode << " completed with: " << result;
			if (NodePingResult::Success != result)
				return;

			if (!IsNodeCompatible(responseNode, networkIdentifier, candidateNode.identityKey())) {
				CATAPULT_LOG(warning) << "ping with " << candidateNode << " rejected due to incompatibility";
				return;
			}

			// if the node responds without a host, use the endpoint that was used to ping it
			if (responseNode.endpoint().Host.empty())
				newPartnerNodeConsumer(ionet::Node(responseNode.identityKey(), candidateNode.endpoint(), responseNode.metadata()));
			else
				newPartnerNodeConsumer(responseNode);
		});
	}
}}
