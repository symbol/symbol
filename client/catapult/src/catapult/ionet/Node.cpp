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

#include "Node.h"
#include "catapult/model/Address.h"
#include "catapult/model/NetworkIdentifier.h"
#include <cctype>

namespace catapult { namespace ionet {

	namespace {
		void MakePrintable(std::string& str) {
			for (auto& ch : str)
				ch = std::isprint(ch) ? ch : '?';
		}

		void CheckStringSize(const char* propertyName, const std::string& str) {
			if (str.size() <= std::numeric_limits<uint8_t>::max())
				return;

			std::ostringstream out;
			out
					<< "cannot create node with " << propertyName << " greater than max size"
					<< std::endl << str << " (size " << str.size() << ")";
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		std::string GetPrintableName(const Key& identityKey, const NodeEndpoint& endpoint, const NodeMetadata& metadata) {
			std::ostringstream printableName;
			if (metadata.Name.empty())
				printableName << model::AddressToString(model::PublicKeyToAddress(identityKey, metadata.NetworkFingerprint.Identifier));
			else
				printableName << metadata.Name;

			if (!endpoint.Host.empty())
				printableName << " @ " + endpoint.Host << ":" << endpoint.Port;

			return printableName.str();
		}
	}

	Node::Node() : Node(model::NodeIdentity())
	{}

	Node::Node(const model::NodeIdentity& identity) : Node(identity, NodeEndpoint(), NodeMetadata())
	{}

	Node::Node(const model::NodeIdentity& identity, const NodeEndpoint& endpoint, const NodeMetadata& metadata)
			: m_identity(identity)
			, m_endpoint(endpoint)
			, m_metadata(metadata) {
		MakePrintable(m_metadata.Name);
		MakePrintable(m_endpoint.Host);

		CheckStringSize("metadata name", m_metadata.Name);
		CheckStringSize("endpoint host", m_endpoint.Host);

		m_printableName = GetPrintableName(m_identity.PublicKey, m_endpoint, m_metadata);
	}

	const model::NodeIdentity& Node::identity() const {
		return m_identity;
	}

	const NodeEndpoint& Node::endpoint() const {
		return m_endpoint;
	}

	const NodeMetadata& Node::metadata() const {
		return m_metadata;
	}

	std::ostream& operator<<(std::ostream& out, const Node& node) {
		out << node.m_printableName;
		return out;
	}
}}
