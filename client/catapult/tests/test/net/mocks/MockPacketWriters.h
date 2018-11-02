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
#include "catapult/net/PacketWriters.h"
#include "tests/test/nodeps/Waits.h"
#include <thread>

namespace catapult { namespace mocks {

	/// Mock packet writers that collects connecting nodes.
	class MockPacketWriters : public net::PacketWriters {
	public:
		size_t numActiveWriters() const override {
			return identities().size();
		}

		utils::KeySet identities() const override {
			utils::KeySet identities;
			for (const auto& node : m_nodes) {
				if (net::PeerConnectCode::Accepted == getResult(node.identityKey()).Code)
					identities.insert(node.identityKey());
			}

			return identities;
		}

		void connect(const ionet::Node& node, const ConnectCallback& callback) override {
			m_nodes.push_back(node);

			// call the callback from a separate thread after some delay
			auto connectResult = getResult(node.identityKey());
			std::thread([callback, connectResult]() {
				test::Pause();
				callback(connectResult);
			}).detach();
		}

		bool closeOne(const Key& identityKey) override {
			m_closedNodeIdentities.insert(identityKey);
			return true;
		}

	public:
		/// Gets connected nodes.
		const std::vector<ionet::Node>& connectedNodes() const {
			return m_nodes;
		}

		/// Gets closed node identities.
		const utils::KeySet& closedNodeIdentities() const {
			return m_closedNodeIdentities;
		}

	public:
		/// Sets the connect code for the node with \a identityKey to \a connectCode.
		void setConnectCode(const Key& identityKey, net::PeerConnectCode connectCode) {
			m_nodeConnectCodeMap.emplace(identityKey, connectCode);
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

		void accept(const std::shared_ptr<ionet::PacketSocket>&, const ConnectCallback&) override {
			CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
		}

		void shutdown() override {
			CATAPULT_THROW_RUNTIME_ERROR("not implemented in mock");
		}

	// endregion

	private:
		net::PeerConnectResult getResult(const Key& identityKey) const {
			auto resultIter = m_nodeConnectCodeMap.find(identityKey);
			return { m_nodeConnectCodeMap.cend() == resultIter ? net::PeerConnectCode::Accepted : resultIter->second, identityKey };
		}

	private:
		std::vector<ionet::Node> m_nodes;
		utils::KeySet m_closedNodeIdentities;
		std::unordered_map<Key, net::PeerConnectCode, utils::ArrayHasher<Key>> m_nodeConnectCodeMap;
	};

	/// Mock packet writers that has a pickOne implementation.
	class PickOneAwareMockPacketWriters : public MockPacketWriters {
	public:
		/// Possible behaviors of setPacketIo.
		enum class SetPacketIoBehavior {
			/// Set packet is only returned once.
			Use_Once,
			/// Set packet is sticky and returned repeatedly.
			Use_Forever
		};

	public:
		/// Creates writers with packet io \a reuse policy.
		explicit PickOneAwareMockPacketWriters(SetPacketIoBehavior setPacketIoBehavior = SetPacketIoBehavior::Use_Forever)
				: m_setPacketIoBehavior(setPacketIoBehavior)
		{}

	public:
		/// Sets the packet io returned by pickOne to \a pPacketIo.
		void setPacketIo(const std::shared_ptr<ionet::PacketIo>& pPacketIo) {
			m_pPacketIo = pPacketIo;
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
			auto pair = ionet::NodePacketIoPair(ionet::Node(), m_pPacketIo);

			// if the io should only be used once, destroy the reference in writers before returning
			if (SetPacketIoBehavior::Use_Once == m_setPacketIoBehavior)
				m_pPacketIo.reset();

			return pair;
		}

	private:
		SetPacketIoBehavior m_setPacketIoBehavior;
		std::vector<utils::TimeSpan> m_ioDurations;
		std::shared_ptr<ionet::PacketIo> m_pPacketIo;
	};

	/// Mock packet writers that has a broadcast implementation.
	class BroadcastAwareMockPacketWriters : public MockPacketWriters {
	public:
		/// Gets the number of broadcast calls.
		size_t numBroadcastCalls() const {
			return m_payloads.size();
		}

		/// Gets the broadcasted payloads.
		const std::vector<ionet::PacketPayload>& broadcastedPayloads() const {
			return m_payloads;
		}

	public:
		void broadcast(const ionet::PacketPayload& payload) override {
			m_payloads.push_back(payload);
		}

	private:
		std::vector<ionet::PacketPayload> m_payloads;
	};
}}
