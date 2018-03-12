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

		using StatsCallback = consumer<const Stats&>;

	public:
		virtual ~PacketSocket() {}

	public:
		/// Retrieves statistics about this socket and passes them to \a callback.
		virtual void stats(const StatsCallback& callback) = 0;

		/// Closes the socket.
		virtual void close() = 0;

		/// Gets a buffered interface to the packet socket.
		virtual std::shared_ptr<PacketIo> buffered() = 0;
	};

	// region Accept

	/// The result of a packet socket accept.
	class AcceptedPacketSocketInfo {
	public:
		/// Creates an empty info.
		AcceptedPacketSocketInfo()
		{}

		/// Creates an info around \a host and \a pPacketSocket.
		explicit AcceptedPacketSocketInfo(const std::string& host, const std::shared_ptr<PacketSocket>& pPacketSocket)
				: m_host(host)
				, m_pPacketSocket(pPacketSocket)
		{}

	public:
		/// Gets the host.
		const std::string& host() const {
			return m_host;
		}

		/// Gets the socket.
		const std::shared_ptr<PacketSocket>& socket() const {
			return m_pPacketSocket;
		}

	public:
		/// Returns \c true if this info is not empty.
		explicit operator bool() const {
			return !!m_pPacketSocket;
		}

	private:
		std::string m_host;
		std::shared_ptr<PacketSocket> m_pPacketSocket;
	};

	/// Callback for configuring a socket before initiating an accept.
	using ConfigureSocketCallback = consumer<socket&>;

	/// Callback for an accepted socket.
	using AcceptCallback = consumer<const AcceptedPacketSocketInfo&>;

	/// Accepts a connection using \a acceptor and calls \a accept on completion configuring the socket with \a options.
	void Accept(boost::asio::ip::tcp::acceptor& acceptor, const PacketSocketOptions& options, const AcceptCallback& accept);

	/// Accepts a connection using \a acceptor and calls \a accept on completion configuring the socket with \a options.
	/// \a configureSocket is called before starting the accept to allow custom configuration of asio sockets.
	/// \note User callbacks passed to the accepted socket are serialized.
	void Accept(
			boost::asio::ip::tcp::acceptor& acceptor,
			const PacketSocketOptions& options,
			const ConfigureSocketCallback& configureSocket,
			const AcceptCallback& accept);

	// endregion

	// region Connect

	/// Callback for a connected socket.
	using ConnectCallback = consumer<ConnectResult, const std::shared_ptr<PacketSocket>&>;

	/// Attempts to connect a socket to the specified \a endpoint using \a service and calls \a callback on
	/// completion configuring the socket with \a options. The returned function can be used to cancel the connect.
	/// \note User callbacks passed to the connected socket are serialized.
	action Connect(
			boost::asio::io_service& service,
			const PacketSocketOptions& options,
			const NodeEndpoint& endpoint,
			const ConnectCallback& callback);

	// endregion
}}
