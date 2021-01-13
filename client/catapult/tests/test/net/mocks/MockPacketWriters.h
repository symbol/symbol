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
#include "catapult/net/PacketWriters.h"
#include "tests/test/nodeps/Waits.h"
#include <thread>
#include <unordered_map>

namespace catapult { namespace mocks {

	/// Mock packet writers that collects connecting nodes.
	class MockPacketWriters : public net::PacketWriters {
	public:
		/// Creates mock packet writers.
		MockPacketWriters()
				: m_equalityStrategy(model::NodeIdentityEqualityStrategy::Key)
				, m_closedNodeIdentities(CreateNodeIdentitySet(m_equalityStrategy))
				, m_nodeConnectCodeMap(model::CreateNodeIdentityMap<net::PeerConnectCode>(m_equalityStrategy))
		{}

	public:
		size_t numActiveWriters() const override {
			return identities().size();
		}

		model::NodeIdentitySet identities() const override {
			auto identities = CreateNodeIdentitySet(m_equalityStrategy);

			for (const auto& node : m_nodes) {
				if (net::PeerConnectCode::Accepted == getResult(node.identity()).Code)
					identities.insert(node.identity());
			}

			return identities;
		}

		void connect(const ionet::Node& node, const ConnectCallback& callback) override {
			m_nodes.push_back(node);

			// call the callback from a separate thread after some delay
			auto connectResult = getResult(node.identity());
			std::thread([callback, connectResult]() {
				test::Pause();
				callback(connectResult);
			}).detach();
		}

		bool closeOne(const model::NodeIdentity& identity) override {
			m_closedNodeIdentities.insert(identity);
			return true;
		}

	public:
		/// Gets the connected nodes.
		const std::vector<ionet::Node>& connectedNodes() const {
			return m_nodes;
		}

		/// Gets the closed node identities.
		const model::NodeIdentitySet& closedNodeIdentities() const {
			return m_closedNodeIdentities;
		}

	public:
		/// Sets the connect code for the node with \a identity to \a connectCode.
		void setConnectCode(const model::NodeIdentity& identity, net::PeerConnectCode connectCode) {
			m_nodeConnectCodeMap.emplace(identity, connectCode);
		}

		/// Immediately marks \a node as connected.
		void connectSync(const ionet::Node& node) {
			m_nodes.push_back(node);
		}

	// region not implemented

	public:
		size_t numActiveConnections() const override {
			CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
		}

		size_t numAvailableWriters() const override {
			CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
		}

		void broadcast(const ionet::PacketPayload&) override {
			CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
		}

		ionet::NodePacketIoPair pickOne(const utils::TimeSpan&) override {
			CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
		}

		void shutdown() override {
			CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
		}

	// endregion

	private:
		net::PeerConnectResultEx getResult(const model::NodeIdentity& identity) const {
			auto resultIter = m_nodeConnectCodeMap.find(identity);
			return {
				m_nodeConnectCodeMap.cend() == resultIter ? net::PeerConnectCode::Accepted : resultIter->second,
				identity,
				nullptr
			};
		}

	private:
		model::NodeIdentityEqualityStrategy m_equalityStrategy;
		std::vector<ionet::Node> m_nodes;
		model::NodeIdentitySet m_closedNodeIdentities;
		model::NodeIdentityMap<net::PeerConnectCode> m_nodeConnectCodeMap;
	};

	/// Mock packet writers that has a pickOne implementation.
	class PickOneAwareMockPacketWriters : public MockPacketWriters {
	public:
		/// Possible behaviors of setPacketIo.
		enum class SetPacketIoBehavior {
			/// Configured packet is only returned once.
			Use_Once,

			/// Configured packet is sticky and returned repeatedly.
			Use_Forever
		};

	public:
		/// Creates writers with packet io \a reuse policy.
		explicit PickOneAwareMockPacketWriters(SetPacketIoBehavior setPacketIoBehavior = SetPacketIoBehavior::Use_Forever)
				: m_setPacketIoBehavior(setPacketIoBehavior)
				, m_nodeIdentity()
		{}

	public:
		/// Sets the packet io returned by pickOne to \a pPacketIo.
		void setPacketIo(const std::shared_ptr<ionet::PacketIo>& pPacketIo) {
			m_pPacketIo = pPacketIo;
		}

		/// Sets the node identity for the node returned by pickOne to \a identity.
		void setNodeIdentity(const model::NodeIdentity& identity) {
			m_nodeIdentity = identity;
		}

	public:
		/// Gets the number of pickOne calls.
		size_t numPickOneCalls() const {
			return m_ioDurations.size();
		}

		/// Gets the durations passed to pickOne.
		const std::vector<utils::TimeSpan>& pickOneDurations() const {
			return m_ioDurations;
		}

	public:
		ionet::NodePacketIoPair pickOne(const utils::TimeSpan& ioDuration) override {
			m_ioDurations.push_back(ioDuration);
			auto pair = ionet::NodePacketIoPair(ionet::Node(m_nodeIdentity), m_pPacketIo);

			// if the io should only be used once, destroy the reference in writers before returning
			if (SetPacketIoBehavior::Use_Once == m_setPacketIoBehavior)
				m_pPacketIo.reset();

			return pair;
		}

	private:
		SetPacketIoBehavior m_setPacketIoBehavior;
		std::vector<utils::TimeSpan> m_ioDurations;
		std::shared_ptr<ionet::PacketIo> m_pPacketIo;
		model::NodeIdentity m_nodeIdentity;
	};

	/// Mock packet writers that has a broadcast implementation.
	class BroadcastAwareMockPacketWriters : public MockPacketWriters {
	public:
		/// Creates writers.
		BroadcastAwareMockPacketWriters() : m_numBroadcastCalls(0)
		{}

	public:
		/// Gets the number of broadcast calls.
		size_t numBroadcastCalls() const {
			return m_numBroadcastCalls;
		}

		/// Gets the broadcasted payloads.
		const std::vector<ionet::PacketPayload>& broadcastedPayloads() const {
			return m_payloads;
		}

	public:
		void broadcast(const ionet::PacketPayload& payload) override {
			m_payloads.push_back(payload);
			++m_numBroadcastCalls;
		}

	private:
		std::atomic<size_t> m_numBroadcastCalls;
		std::vector<ionet::PacketPayload> m_payloads;
	};
}}
