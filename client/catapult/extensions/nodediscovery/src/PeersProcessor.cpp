/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "PeersProcessor.h"
#include "NodePingUtils.h"
#include "catapult/ionet/NodeContainer.h"

namespace catapult { namespace nodediscovery {

	PeersProcessor::PeersProcessor(
			const Key& serverPublicKey,
			const ionet::NodeContainer& nodeContainer,
			const NodePingRequestInitiator& pingRequestInitiator,
			const model::UniqueNetworkFingerprint& networkFingerprint,
			const NodeConsumer& newPartnerNodeConsumer)
			: m_serverPublicKey(serverPublicKey)
			, m_nodeContainer(nodeContainer)
			, m_pingRequestInitiator(pingRequestInitiator)
			, m_networkFingerprint(networkFingerprint)
			, m_newPartnerNodeConsumer(newPartnerNodeConsumer)
	{}

	void PeersProcessor::process(const ionet::NodeSet& candidateNodes) const {
		for (const auto& candidateNode : SelectUnknownNodes(m_nodeContainer.view(), candidateNodes)) {
			// compare identity keys to bypass self pings even when local host is loopback address
			if (m_serverPublicKey == candidateNode.identity().PublicKey) {
				CATAPULT_LOG(debug) << "bypassing ping with local node: " << candidateNode;
				continue;
			}

			CATAPULT_LOG(debug) << "initiating ping with: " << candidateNode;
			process(candidateNode);
		}
	}

	void PeersProcessor::process(const ionet::Node& candidateNode) const {
		auto networkFingerprint = m_networkFingerprint;
		auto newPartnerNodeConsumer = m_newPartnerNodeConsumer;
		m_pingRequestInitiator(candidateNode, [candidateNode, networkFingerprint, newPartnerNodeConsumer](
				auto result,
				const auto& responseNode) {
			CATAPULT_LOG(info) << "ping with '" << candidateNode << "' completed with: " << result;
			if (net::NodeRequestResult::Success != result)
				return;

			if (!IsNodeCompatible(responseNode, networkFingerprint, candidateNode.identity().PublicKey)) {
				CATAPULT_LOG(warning) << "ping with '" << candidateNode << "' rejected due to incompatibility";
				return;
			}

			// if the node responds without a host, use the endpoint that was used to ping it
			if (responseNode.endpoint().Host.empty())
				newPartnerNodeConsumer(ionet::Node(responseNode.identity(), candidateNode.endpoint(), responseNode.metadata()));
			else
				newPartnerNodeConsumer(responseNode);
		});
	}
}}
