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
#include "catapult/ionet/NodeContainer.h"
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
		NodeSubscriberBanParams(const model::NodeIdentity& identity, uint32_t reason)
				: Identity(identity)
				, Reason(reason)
		{}

	public:
		/// Banned node identity.
		const model::NodeIdentity Identity;

		/// Ban reason.
		const uint32_t Reason;
	};

	/// Mock node subscriber implementation.
	class MockNodeSubscriber : public subscribers::NodeSubscriber {
	public:
		/// Creates a mock subscriber.
		MockNodeSubscriber() : MockNodeSubscriber(nullptr)
		{}

		/// Creates a mock subscriber around \a nodes.
		explicit MockNodeSubscriber(ionet::NodeContainer& nodes) : MockNodeSubscriber(&nodes)
		{}

	private:
		MockNodeSubscriber(ionet::NodeContainer* pNodes)
				: m_pNodes(pNodes)
				, m_numNodesNotified(0)
				, m_enableBanSimulation(false)
				, m_notifyIncomingNodeResult(true)
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
		/// Enables banning on the node container when ban notifications are made.
		void enableBanSimulation() {
			m_enableBanSimulation = true;
		}

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

		void notifyBan(const model::NodeIdentity& identity, uint32_t reason) override {
			m_banParams.push(identity, reason);

			if (m_enableBanSimulation)
				m_pNodes->modifier().ban(identity, reason);
		}

	private:
		ionet::NodeContainer* m_pNodes;
		std::atomic<size_t> m_numNodesNotified;
		bool m_enableBanSimulation;
		bool m_notifyIncomingNodeResult;

		test::ParamsCapture<NodeSubscriberNodeParams> m_nodeParams;
		test::ParamsCapture<NodeSubscriberIncomingNodeParams> m_incomingNodeParams;
		test::ParamsCapture<NodeSubscriberBanParams> m_banParams;
	};
}}
