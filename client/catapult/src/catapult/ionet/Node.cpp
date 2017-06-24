#include "Node.h"
#include "catapult/model/Address.h"
#include "catapult/model/NetworkInfo.h"
#include <cctype>

namespace catapult { namespace ionet {

	namespace {
		std::string GetPrintableName(const NodeIdentity& identity, model::NetworkIdentifier networkIdentifier) {
			if (identity.Name.empty())
				return model::AddressToString(model::PublicKeyToAddress(identity.PublicKey, networkIdentifier));

			auto i = 0u;
			std::string printableName(identity.Name.size(), '?');
			for (auto ch : identity.Name)
				printableName[i++] = std::isprint(ch) ? ch : '?';

			return printableName;
		}

		std::string GetEndpointPostfix(const NodeEndpoint& endpoint) {
			return endpoint.Host.empty() ? "" : " @ " + endpoint.Host;
		}
	}

	Node::Node() : Node(NodeEndpoint(), NodeIdentity(), model::NetworkIdentifier::Zero)
	{}

	Node::Node(const NodeEndpoint& endpoint, const NodeIdentity& identity, model::NetworkIdentifier networkIdentifier)
			: Endpoint(endpoint)
			, Identity(identity)
			, NetworkIdentifier(networkIdentifier)
			, m_printableName(GetPrintableName(Identity, networkIdentifier) + GetEndpointPostfix(Endpoint))
	{}

	bool Node::operator==(const Node& rhs) const {
		return Identity.PublicKey == rhs.Identity.PublicKey && NetworkIdentifier == rhs.NetworkIdentifier;
	}

	bool Node::operator!=(const Node& rhs) const {
		return !(*this == rhs);
	}

	std::ostream& operator<<(std::ostream& out, const Node& node) {
		out << node.m_printableName;
		return out;
	}
}}
