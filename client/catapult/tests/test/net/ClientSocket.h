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
#include "catapult/ionet/IoTypes.h"
#include "catapult/thread/Future.h"
#include <functional>

namespace catapult { namespace test {

	/// Wraps a client, non packet-based socket.
	class ClientSocket {
	public:
		/// Connect options.
		enum class ConnectOptions {
			/// Normal connect behavior.
			Normal,

			/// Abort connection immediately after connect.
			Abort,

			/// Skip handshake after connect.
			Skip_Handshake
		};

	public:
		virtual ~ClientSocket() = default;

	public:
		/// Connects to the (localhost) server with \a options.
		virtual thread::future<ClientSocket*> connect(ConnectOptions options = ConnectOptions::Normal) = 0;

		/// Connects to the (localhost) server at \a port with \a options.
		virtual thread::future<ClientSocket*> connect(unsigned short port, ConnectOptions options = ConnectOptions::Normal) = 0;

		/// Reads a buffer from this socket into \a receiveBuffer.
		virtual thread::future<size_t> read(ionet::ByteBuffer& receiveBuffer) = 0;

		/// Writes \a sendBuffer to this \a socket.
		virtual thread::future<size_t> write(const ionet::ByteBuffer& sendBuffer) = 0;

		/// Writes all buffers in \a sendBuffers to this socket with an optional delay (\a delayMillis) between writes.
		virtual thread::future<size_t> write(const std::vector<ionet::ByteBuffer>& sendBuffers, size_t delayMillis = 10) = 0;

		/// Delays execution of \a continuation by \a delayMillis milliseconds.
		virtual void delay(const action& continuation, size_t delayMillis) = 0;

		/// Shuts down the client socket.
		virtual void shutdown() = 0;

		/// Aborts the client socket.
		virtual void abort() = 0;
	};

	/// Creates a client socket around \a ioContext.
	std::shared_ptr<ClientSocket> CreateClientSocket(boost::asio::io_context& ioContext);

	/// Spawns a task on \a ioContext that connects to the (localhost) server.
	std::shared_ptr<ClientSocket> AddClientConnectionTask(boost::asio::io_context& ioContext);

	/// Spawns a task on \a ioContext that reads \a receiveBuffer from a client socket.
	std::shared_ptr<ClientSocket> AddClientReadBufferTask(boost::asio::io_context& ioContext, ionet::ByteBuffer& receiveBuffer);

	/// Spawns a task on \a ioContext that writes all \a sendBuffers to a client socket.
	std::shared_ptr<ClientSocket> AddClientWriteBuffersTask(
			boost::asio::io_context& ioContext,
			const std::vector<ionet::ByteBuffer>& sendBuffers);
}}
