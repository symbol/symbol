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

#pragma once
#include "catapult/ionet/NodeSet.h"
#include "catapult/net/NodeRequestResult.h"
#include "catapult/functions.h"

namespace catapult { namespace ionet { class NodeContainer; } }

namespace catapult { namespace nodediscovery {

	/// Processes peers received from a partner node.
	class PeersProcessor {
	private:
		using NodeConsumer = consumer<const ionet::Node&>;
		using NodePingRequestInitiator = consumer<const ionet::Node&, const consumer<net::NodeRequestResult, const ionet::Node&>&>;

	public:
		/// Creates a processor around the server public key (\a serverPublicKey), the set of known nodes (\a nodeContainer),
		/// a service for pinging other nodes (\a pingRequesInitiator), the network fingerprint (\a networkFingerprint)
		/// and a consumer that should be called when new partner nodes are discovered (\a newPartnerNodeConsumer).
		PeersProcessor(
				const Key& serverPublicKey,
				const ionet::NodeContainer& nodeContainer,
				const NodePingRequestInitiator& pingRequestInitiator,
				const model::UniqueNetworkFingerprint& networkFingerprint,
				const NodeConsumer& newPartnerNodeConsumer);

	public:
		/// Processes all candidate nodes (\a candidateNodes) and forwards newly discovered partner nodes.
		void process(const ionet::NodeSet& candidateNodes) const;

	private:
		void process(const ionet::Node& candidateNode) const;

	private:
		const Key& m_serverPublicKey;
		const ionet::NodeContainer& m_nodeContainer;
		NodePingRequestInitiator m_pingRequestInitiator;
		model::UniqueNetworkFingerprint m_networkFingerprint;
		NodeConsumer m_newPartnerNodeConsumer;
	};
}}
