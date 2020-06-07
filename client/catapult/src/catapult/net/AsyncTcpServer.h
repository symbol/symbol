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
#include "catapult/ionet/PacketSocket.h"

namespace catapult { namespace thread { class IoThreadPool; } }

namespace catapult { namespace net {

	using AcceptHandler = consumer<const ionet::PacketSocketInfo&>;

	/// Settings used to configure AsyncTcpServer behavior.
	struct AsyncTcpServerSettings {
	public:
		/// Creates a structure with a preconfigured accept handler (\a accept).
		explicit AsyncTcpServerSettings(const AcceptHandler& accept);

	public:
		/// Accept handler (must be set via constructor).
		const AcceptHandler Accept;

		/// Packet socket options.
		ionet::PacketSocketOptions PacketSocketOptions;

		/// Maximum number of pending connections (backlog size).
		uint16_t MaxPendingConnections = 100;

		/// Maximum number of active connections.
		uint32_t MaxActiveConnections = 25;

		/// \c true if the server should reuse ports already in use.
		bool AllowAddressReuse = false;
	};

	/// Async TCP server.
	class AsyncTcpServer {
	public:
		virtual ~AsyncTcpServer() = default;

	public:
		/// Number of asynchronously started (but not completed) socket accepts.
		virtual uint32_t numPendingAccepts() const = 0;

		/// Current number of active connections.
		virtual uint32_t numCurrentConnections() const = 0;

		/// Total number of connections during the server's lifetime.
		virtual uint32_t numLifetimeConnections() const = 0;

	public:
		/// Shuts down the server.
		virtual void shutdown() = 0;
	};

	/// Creates an async tcp server listening on \a endpoint with the specified \a settings using the specified thread \a pool.
	std::shared_ptr<AsyncTcpServer> CreateAsyncTcpServer(
			thread::IoThreadPool& pool,
			const boost::asio::ip::tcp::endpoint& endpoint,
			const AsyncTcpServerSettings& settings);
}}
