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
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/NodePacketIoPair.h"
#include "catapult/thread/Future.h"
#include "catapult/utils/SpinLock.h"

namespace catapult {
	namespace net { class PacketWriters; }
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace tools {

	/// Class that holds connections to network peers.
	class NetworkConnections {
	public:
		/// Creates a network connections object around \a nodes.
		explicit NetworkConnections(const std::vector<ionet::Node>& nodes);

		/// Move constructor.
		NetworkConnections(NetworkConnections&& rhs) = default;

		/// Destroys the connections.
		~NetworkConnections();

	public:
		/// Gets the number of active connections (including pending connections).
		size_t numActiveConnections() const;

		/// Gets the number of active writers.
		size_t numActiveWriters() const;

		/// Gets the number of available writers.
		/// \note There will be fewer available writers than active writers when some writers are checked out.
		size_t numAvailableWriters() const;

	public:
		/// Establishes connections to all peers in the network.
		thread::future<bool> connectAll();

		/// Establishes connections to all disconnected peers in the network.
		thread::future<bool> connectDisconnected();

		/// Picks one network connection.
		ionet::NodePacketIoPair pickOne() const;

		/// Adds \a node to the set of disconnected nodes.
		void addDisconnected(const ionet::Node& node);

	private:
		void removeDisconnected(const ionet::Node& node);

		thread::future<bool> connectAll(const std::vector<ionet::Node>& nodes);

		void shutdown();

	private:
		std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
		std::unique_ptr<crypto::KeyPair> m_pClientKeyPair;
		const std::vector<ionet::Node> m_nodes;
		ionet::NodeSet m_disconnectedNodes;
		std::shared_ptr<net::PacketWriters> m_pPacketWriters;
		std::unique_ptr<utils::SpinLock> m_pSpinLock;
	};

	/// Gets the current network height using \a connections.
	Height GetHeight(const NetworkConnections& connections);

	/// Waits for the network to produce \a numBlocks blocks using \a connections.
	bool WaitForBlocks(const NetworkConnections& connections, size_t numBlocks);
}}
