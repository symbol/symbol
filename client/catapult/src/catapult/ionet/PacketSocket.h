#pragma once
#include "BatchPacketReader.h"
#include "ConnectResult.h"
#include "IoTypes.h"
#include "PacketSocketOptions.h"

namespace catapult {
	namespace ionet {
		struct NodeEndpoint;
		struct Packet;
	}
}

namespace catapult { namespace ionet {

	/// An asio socket wrapper that natively supports packets.
	/// This wrapper is threadsafe but does not prevent interleaving reads or writes.
	class PacketSocket : public PacketIo, public BatchPacketReader {
	public:
		/// Statistics about a socket.
		struct Stats {
			/// \c true if the socket is open.
			bool IsOpen;

			/// The number of unprocessed bytes.
			size_t NumUnprocessedBytes;
		};

		using StatsCallback = std::function<void (const Stats&)>;

	public:
		virtual ~PacketSocket() {}

	public:
		/// Retrieves statistics about this socket and passes them to \a callback.
		virtual void stats(const StatsCallback& callback) = 0;

		/// Closes the socket.
		virtual void close() = 0;
	};

	using ConfigureSocketCallback = std::function<void (socket&)>;
	using AcceptCallback = std::function<void (const std::shared_ptr<PacketSocket>&)>;
	using ConnectCallback = std::function<void (ConnectResult, const std::shared_ptr<PacketSocket>&)>;

	/// Accepts a connection using \a acceptor and calls \a accept on completion configuring the socket with \a options.
	void Accept(
			boost::asio::ip::tcp::acceptor& acceptor,
			const PacketSocketOptions& options,
			const AcceptCallback& accept);

	/// Accepts a connection using \a acceptor and calls \a accept on completion configuring the socket with \a options.
	/// \a configureSocket is called before starting the accept to allow custom configuration of asio sockets.
	/// \note User callbacks passed to the accepted socket are serialized.
	void Accept(
			boost::asio::ip::tcp::acceptor& acceptor,
			const PacketSocketOptions& options,
			const ConfigureSocketCallback& configureSocket,
			const AcceptCallback& accept);

	/// Attempts to connect a socket to the specified \a endpoint using \a service and calls \a callback on
	/// completion configuring the socket with \a options. The returned function can be used to cancel the connect.
	/// \note User callbacks passed to the connected socket are serialized.
	std::function<void ()> Connect(
			boost::asio::io_service& service,
			const PacketSocketOptions& options,
			const NodeEndpoint& endpoint,
			const ConnectCallback& callback);
}}
