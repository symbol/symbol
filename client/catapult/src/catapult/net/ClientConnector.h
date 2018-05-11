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
#include "ConnectionSettings.h"
#include "PeerConnectResult.h"
#include "catapult/functions.h"
#include <memory>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace ionet { class PacketSocket; }
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace net {

	/// Accepts connections that are initiated by external nodes to this (local) node.
	class ClientConnector {
	public:
		/// A callback that is passed the accept result as well as the client socket and public key (on success).
		using AcceptCallback = consumer<PeerConnectResult, const std::shared_ptr<ionet::PacketSocket>&, const Key&>;

	public:
		virtual ~ClientConnector() {}

	public:
		/// Gets the number of active connections.
		virtual size_t numActiveConnections() const = 0;

	public:
		/// Accepts a connection represented by \a pAcceptedSocket and calls \a callback on completion.
		virtual void accept(const std::shared_ptr<ionet::PacketSocket>& pAcceptedSocket, const AcceptCallback& callback) = 0;

		/// Shuts down all connections.
		virtual void shutdown() = 0;
	};

	/// Creates a client connector for a server with a key pair of \a keyPair using \a pPool and configured with \a settings.
	std::shared_ptr<ClientConnector> CreateClientConnector(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings);
}}
