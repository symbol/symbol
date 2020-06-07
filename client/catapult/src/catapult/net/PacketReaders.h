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
#include "PeerConnectResult.h"
#include "catapult/ionet/PacketHandlers.h"
#include <functional>
#include <memory>

namespace catapult {
	namespace ionet {
		class PacketIo;
		class PacketSocket;
		class PacketSocketInfo;
	}
	namespace thread { class IoThreadPool; }
}

namespace catapult { namespace net {

	/// Manages a collection of connections that receive data from external nodes.
	class PacketReaders : public AcceptedConnectionContainer {
	public:
		/// Gets the number of active readers.
		virtual size_t numActiveReaders() const = 0;

	public:
		/// Shuts down all connections.
		virtual void shutdown() = 0;
	};

	/// Creates a packet readers container for a server with specified \a serverPublicKey using \a pool and \a handlers,
	/// configured with \a settings and allowing \a maxConnectionsPerIdentity.
	std::shared_ptr<PacketReaders> CreatePacketReaders(
			thread::IoThreadPool& pool,
			const ionet::ServerPacketHandlers& handlers,
			const Key& serverPublicKey,
			const ConnectionSettings& settings,
			uint32_t maxConnectionsPerIdentity);
}}
