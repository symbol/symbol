#include "Node.h"
#include "catapult/model/Address.h"
#include "catapult/model/NetworkInfo.h"
#include <cctype>

namespace catapult { namespace ionet {

	namespace {
		void MakePrintable(std::string& str) {
			for (auto& ch : str)
				ch = std::isprint(ch) ? ch : '?';
		}

		std::string GetPrintableName(const Key& identityKey, const NodeEndpoint& endpoint, const NodeMetadata& metadata) {
			auto printableName = metadata.Name.empty()
					? model::AddressToString(model::PublicKeyToAddress(identityKey, metadata.NetworkIdentifier))
					: metadata.Name;

			if (!endpoint.Host.empty())
				printableName += " @ " + endpoint.Host;

			return printableName;
		}
	}

	Node::Node() : Node(Key(), NodeEndpoint(), NodeMetadata())
	{}

	Node::Node(const Key& identityKey, const NodeEndpoint& endpoint, const NodeMetadata& metadata)
			: m_identityKey(identityKey)
			, m_endpoint(endpoint)
			, m_metadata(metadata) {
		MakePrintable(m_metadata.Name);
		MakePrintable(m_endpoint.Host);
		m_printableName = GetPrintableName(m_identityKey, m_endpoint, m_metadata);
	}

	const Key& Node::identityKey() const {
		return m_identityKey;
	}

	const NodeEndpoint& Node::endpoint() const {
		return m_endpoint;
	}

	const NodeMetadata& Node::metadata() const {
		return m_metadata;
	}

	bool Node::operator==(const Node& rhs) const {
		return m_identityKey == rhs.m_identityKey && m_metadata.NetworkIdentifier == rhs.m_metadata.NetworkIdentifier;
	}

	bool Node::operator!=(const Node& rhs) const {
		return !(*this == rhs);
	}

	std::ostream& operator<<(std::ostream& out, const Node& node) {
		out << node.m_printableName;
		return out;
	}
}}
