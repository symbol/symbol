#pragma once
#include "catapult/ionet/IoTypes.h"
#include "catapult/thread/Future.h"
#include <functional>

namespace catapult { namespace test {

	/// Wraps a client, non packet-based socket.
	class ClientSocket {
	public:
		virtual ~ClientSocket() {}

	public:
		/// Connects to the (localhost) server.
		virtual thread::future<ClientSocket*> connect() = 0;

		/// Connects to the (localhost) server at \a port.
		virtual thread::future<ClientSocket*> connect(unsigned short port) = 0;

		/// Reads a buffer from this socket into \a receiveBuffer.
		virtual thread::future<size_t> read(ionet::ByteBuffer& receiveBuffer) = 0;

		/// Writes \a sendBuffer to this \a socket.
		virtual thread::future<size_t> write(const ionet::ByteBuffer& sendBuffer) = 0;

		/// Writes all buffers in \a sendBuffers to this socket with an optional delay (\a delayMillis) between writes.
		virtual thread::future<size_t> write(
				const std::vector<ionet::ByteBuffer>& sendBuffers,
				size_t delayMillis = 10) = 0;

		/// Delays execution of \a continuation by \a delayMillis milliseconds.
		virtual void delay(const std::function<void ()>& continuation, size_t delayMillis) = 0;

		/// Shutdowns the client socket.
		virtual void shutdown() = 0;
	};

	/// Creates a client socket around \a service.
	std::shared_ptr<ClientSocket> CreateClientSocket(boost::asio::io_service& service);

	/// Spawns a task on \a service that connects to the (localhost) server.
	void AddClientConnectionTask(boost::asio::io_service& service);

	/// Spawns a task on \a service that reads \a receiveBuffer from a client socket.
	void AddClientReadBufferTask(boost::asio::io_service& service, ionet::ByteBuffer& receiveBuffer);

	/// Spawns a task on \a service that writes all \a sendBuffers to a client socket.
	void AddClientWriteBuffersTask(
			boost::asio::io_service& service,
			const std::vector<ionet::ByteBuffer>& sendBuffers);
}}
