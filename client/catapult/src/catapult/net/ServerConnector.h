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
#include "PeerConnectCode.h"
#include "catapult/functions.h"
#include <memory>

namespace catapult {
	namespace ionet {
		class Node;
		class PacketSocket;
		class PacketSocketInfo;
	}
	namespace thread { class IoThreadPool; }
}

namespace catapult { namespace net {

	/// Establishes connections with external nodes that this (local) node initiates.
	class ServerConnector {
	public:
		/// Callback that is passed the connect result and the connected socket info on success.
		using ConnectCallback = consumer<PeerConnectCode, const ionet::PacketSocketInfo&>;

	public:
		virtual ~ServerConnector() = default;

	public:
		/// Gets the number of active connections.
		virtual size_t numActiveConnections() const = 0;

		/// Gets the friendly name of this connector.
		virtual const std::string& name() const = 0;

	public:
		/// Attempts to connect to \a node and calls \a callback on completion.
		virtual void connect(const ionet::Node& node, const ConnectCallback& callback) = 0;

		/// Shuts down all connections.
		virtual void shutdown() = 0;
	};

	/// Creates a server connector for a server with specified \a serverPublicKey using \a pool and configured with \a settings.
	/// Optional friendly \a name can be provided to tag logs.
	std::shared_ptr<ServerConnector> CreateServerConnector(
			thread::IoThreadPool& pool,
			const Key& serverPublicKey,
			const ConnectionSettings& settings,
			const char* name = nullptr);
}}
