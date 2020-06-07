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
#include "ConnectionContainer.h"
#include "ConnectionSettings.h"
#include "PacketIoPicker.h"
#include "PeerConnectResult.h"
#include <memory>

namespace catapult {
	namespace ionet {
		class Node;
		class PacketSocket;
	}
	namespace net { struct Packet; }
	namespace thread { class IoThreadPool; }
}

namespace catapult { namespace net {

	/// Manages a collection of connections that send data to external nodes.
	class PacketWriters : public ConnectionContainer, public PacketIoPicker {
	public:
		using ConnectCallback = consumer<const PeerConnectResultEx&>;

	public:
		/// Gets the number of active writers.
		virtual size_t numActiveWriters() const = 0;

		/// Gets the number of available writers.
		/// \note There will be fewer available writers than active writers when some writers are checked out.
		virtual size_t numAvailableWriters() const = 0;

	public:
		/// Broadcasts \a payload to all active connections.
		virtual void broadcast(const ionet::PacketPayload& payload) = 0;

	public:
		/// Attempts to connect to \a node and calls \a callback on completion.
		virtual void connect(const ionet::Node& node, const ConnectCallback& callback) = 0;

		/// Shuts down all connections.
		virtual void shutdown() = 0;
	};

	/// Creates a packet writers container for a server with specified \a serverPublicKey using \a pool and configured with
	/// \a settings.
	std::shared_ptr<PacketWriters> CreatePacketWriters(
			thread::IoThreadPool& pool,
			const Key& serverPublicKey,
			const ConnectionSettings& settings);
}}
