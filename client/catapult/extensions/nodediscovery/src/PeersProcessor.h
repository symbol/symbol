#pragma once
#include "catapult/ionet/Node.h"
#include "catapult/functions.h"

namespace catapult {
	namespace ionet { class NodeContainer; }
	namespace nodediscovery { class NodePingRequestor; }
}

namespace catapult { namespace nodediscovery {

	/// Processes peers received from a partner node.
	class PeersProcessor {
	private:
		using NodeConsumer = consumer<const ionet::Node&>;

	public:
		/// Creates a processor around the set of known nodes (\a nodeContainer), a service for pinging other nodes (\a pingRequestor),
		/// the current network identifier (\a networkIdentifier) and a callback that should be called when new partner nodes
		/// are discovered (\a newPartnerNodeConsumer).
		PeersProcessor(
				const ionet::NodeContainer& nodeContainer,
				NodePingRequestor& pingRequestor,
				model::NetworkIdentifier networkIdentifier,
				const NodeConsumer& newPartnerNodeConsumer);

	public:
		/// Processes all candidate nodes (\a candidateNodes) and forwards newly discovered partner nodes.
		void process(const ionet::NodeSet& candidateNodes) const;

	private:
		void process(const ionet::Node& candidateNode) const;

	private:
		const ionet::NodeContainer& m_nodeContainer;
		NodePingRequestor& m_pingRequestor;
		model::NetworkIdentifier m_networkIdentifier;
		NodeConsumer m_newPartnerNodeConsumer;
	};
}}
