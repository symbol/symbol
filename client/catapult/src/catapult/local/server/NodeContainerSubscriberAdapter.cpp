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

#include "NodeContainerSubscriberAdapter.h"
#include "catapult/config/NodeConfiguration.h"
#include "catapult/ionet/NodeContainer.h"

namespace catapult { namespace local {

	namespace {
		class NodeContainerSubscriberAdapter : public subscribers::NodeSubscriber {
		public:
			NodeContainerSubscriberAdapter(ionet::NodeContainer& nodes, const std::unordered_set<std::string>& localNetworks)
					: m_nodes(nodes)
					, m_localNetworks(localNetworks)
			{}

		public:
			void notifyNode(const ionet::Node& node) override {
				m_nodes.modifier().add(node, ionet::NodeSource::Dynamic);
			}

			bool notifyIncomingNode(const model::NodeIdentity& identity, ionet::ServiceIdentifier serviceId) override {
				auto adjustedHost = config::IsLocalHost(identity.Host, m_localNetworks) ? "127.0.0.1" : identity.Host;
				ionet::Node node({ identity.PublicKey, adjustedHost }, { adjustedHost, 0 }, ionet::NodeMetadata());

				auto modifier = m_nodes.modifier();
				if (modifier.add(node, ionet::NodeSource::Dynamic_Incoming)) {
					++modifier.provisionConnectionState(serviceId, node.identity()).Age;
					return true;
				}

				CATAPULT_LOG(warning) << "could not add incoming node (" << identity << ") to node container";
				return false;
			}

			void notifyBan(const model::NodeIdentity& identity, validators::ValidationResult reason) override {
				m_nodes.modifier().ban(identity, utils::to_underlying_type(reason));
			}

		private:
			ionet::NodeContainer& m_nodes;
			std::unordered_set<std::string> m_localNetworks;
		};
	}

	std::unique_ptr<subscribers::NodeSubscriber> CreateNodeContainerSubscriberAdapter(
			ionet::NodeContainer& nodes,
			const std::unordered_set<std::string>& localNetworks) {
		return std::make_unique<NodeContainerSubscriberAdapter>(nodes, localNetworks);
	}
}}
