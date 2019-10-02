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
#include "catapult/model/NodeIdentity.h"
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
		/// Creates params around \a identity and \a serviceId.
		NodeSubscriberIncomingNodeParams(const model::NodeIdentity& identity, ionet::ServiceIdentifier serviceId)
				: Identity(identity)
				, ServiceId(serviceId)
		{}

	public:
		/// Incoming node identity.
		const model::NodeIdentity Identity;

		/// Incoming service identifier.
		const ionet::ServiceIdentifier ServiceId;
	};

	/// Node subscriber ban params.
	struct NodeSubscriberBanParams {
	public:
		/// Creates params around \a identity and \a reason.
		NodeSubscriberBanParams(const model::NodeIdentity& identity, validators::ValidationResult reason)
				: Identity(identity)
				, Reason(reason)
		{}

	public:
		/// Banned node identity.
		const model::NodeIdentity Identity;

		/// Ban reason.
		const validators::ValidationResult Reason;
	};

	/// Mock noop node subscriber implementation.
	class MockNodeSubscriber : public subscribers::NodeSubscriber {
	public:
		/// Creates a mock subscriber.
		MockNodeSubscriber() : m_numNodesNotified(0)
		{}

	public:
		/// Gets the number of times notifyNode was called (in a threadsafe manner).
		size_t numNodesNotified() const {
			return m_numNodesNotified;
		}

	public:
		/// Gets the params passed to notifyNode.
		const auto& nodeParams() const {
			return m_nodeParams;
		}

		/// Gets the params passed to notifyIncomingNode.
		const auto& incomingNodeParams() const {
			return m_incomingNodeParams;
		}

		/// Gets the params passed to notifyBan.
		const auto& banParams() const {
			return m_banParams;
		}

	public:
		/// Sets the value returned by notifyIncomingNode to \a result.
		void setNotifyIncomingNodeResult(bool result) {
			m_notifyIncomingNodeResult = result;
		}

	public:
		void notifyNode(const ionet::Node& node) override {
			m_nodeParams.push(node);
			++m_numNodesNotified;
		}

		bool notifyIncomingNode(const model::NodeIdentity& identity, ionet::ServiceIdentifier serviceId) override {
			m_incomingNodeParams.push(identity, serviceId);
			return m_notifyIncomingNodeResult;
		}

		void notifyBan(const model::NodeIdentity& identity, validators::ValidationResult reason) override {
			m_banParams.push(identity, reason);
		}

	private:
		bool m_notifyIncomingNodeResult = true;
		test::ParamsCapture<NodeSubscriberNodeParams> m_nodeParams;
		test::ParamsCapture<NodeSubscriberIncomingNodeParams> m_incomingNodeParams;
		test::ParamsCapture<NodeSubscriberBanParams> m_banParams;
		std::atomic<size_t> m_numNodesNotified;
	};
}}
