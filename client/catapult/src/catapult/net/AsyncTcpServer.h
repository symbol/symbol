#pragma once
#include "catapult/ionet/IoTypes.h"
#include "catapult/ionet/PacketSocket.h"
#include <functional>
#include <memory>

namespace catapult { namespace thread { class IoServiceThreadPool; } }

namespace catapult { namespace net {

	/// The structure passed to the accept handler callback.
	class AsyncTcpServerAcceptContext {
	public:
		virtual ~AsyncTcpServerAcceptContext() {}

	public:
		/// Gets the underlying io_service associated with the socket.
		virtual boost::asio::io_service& service() = 0;

		/// Gets the accepted socket.
		/// \remark This is a shared pointer instead of a reference so that aggregates (like readers and writers)
		/// can keep the socket object alive until they complete. Regardless of outstanding socket references,
		/// the socket is closed when the owning AsyncTcpServerAcceptContext is destroyed.
		virtual std::shared_ptr<ionet::PacketSocket> socket() = 0;
	};

	using AcceptHandler = std::function<void (const std::shared_ptr<AsyncTcpServerAcceptContext>& pContext)>;

	using ConfigureSocketHandler = std::function<void (ionet::socket&)>;

	/// Settings used to configure AsyncTcpServer behavior.
	struct AsyncTcpServerSettings {
	public:
		/// Creates a structure with a preconfigured accept handler (\a accept).
		AsyncTcpServerSettings(const AcceptHandler& accept);

	public:
		/// The accept handler (must be set via constructor).
		const AcceptHandler Accept;

		// The configure socket handler.
		ConfigureSocketHandler ConfigureSocket;

		/// Packet socket options.
		ionet::PacketSocketOptions PacketSocketOptions;

		// The maximum number of connections.
		int MaxConnections = 100;

		// The maximum number of active connections.
		uint32_t MaxActiveConnections = 25;

		/// \c true if the server should reuse ports already in use.
		bool AllowAddressReuse = false;
	};

	/// An async TCP server.
	class AsyncTcpServer {
	public:
		virtual ~AsyncTcpServer() {}

	public:
		/// The number of asynchronously started (but not completed) socket accepts.
		virtual uint32_t numPendingAccepts() const = 0;

		/// The current number of active connections.
		virtual uint32_t numCurrentConnections() const = 0;

		/// The total number of connections during the server's lifetime.
		virtual uint32_t numLifetimeConnections() const = 0;

	public:
		/// Shutdowns the server.
		virtual void shutdown() = 0;
	};

	/// Creates an async tcp server listening on \a endpoint with the specified \a settings using the specified
	/// thread pool (\a pPool).
	std::shared_ptr<AsyncTcpServer> CreateAsyncTcpServer(
		const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
		const boost::asio::ip::tcp::endpoint& endpoint,
		const AsyncTcpServerSettings& settings);
}}
