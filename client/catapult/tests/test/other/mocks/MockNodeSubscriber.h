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

#pragma once
#include "catapult/subscribers/NodeSubscriber.h"
#include "tests/test/nodeps/ParamsCapture.h"

namespace catapult { namespace mocks {

	/// Node subscriber node params.
	struct NodeSubscriberNodeParams {
	public:
		/// Creates params around \a node.
		explicit NodeSubscriberNodeParams(const ionet::Node& node)
				: Node(node)
				, NodeCopy(node)
		{}

	public:
		/// Reference to the node.
		const ionet::Node& Node;

		/// Copy of the node.
		const ionet::Node NodeCopy;
	};

	/// Node subscriber incoming node params.
	struct NodeSubscriberIncomingNodeParams {
	public:
		/// Creates params around \a identityKey and \a serviceId.
		explicit NodeSubscriberIncomingNodeParams(const Key& identityKey, ionet::ServiceIdentifier serviceId)
				: IdentityKey(identityKey)
				, ServiceId(serviceId)
		{}

	public:
		/// Incoming node identity.
		const Key IdentityKey;

		/// Incoming service identifier.
		const ionet::ServiceIdentifier ServiceId;
	};

	/// Mock noop node subscriber implementation.
	class MockNodeSubscriber : public subscribers::NodeSubscriber {
	public:
		/// Gets params passed to notifyNode.
		const auto& nodeParams() const {
			return m_nodeParams;
		}

		/// Gets params passed to notifyIncomingNode.
		const auto& incomingNodeParams() const {
			return m_incomingNodeParams;
		}

	public:
		void notifyNode(const ionet::Node& node) override {
			m_nodeParams.push(node);
		}

		void notifyIncomingNode(const Key& identityKey, ionet::ServiceIdentifier serviceId) override {
			m_incomingNodeParams.push(identityKey, serviceId);
		}

	private:
		test::ParamsCapture<NodeSubscriberNodeParams> m_nodeParams;
		test::ParamsCapture<NodeSubscriberIncomingNodeParams> m_incomingNodeParams;
	};
}}
