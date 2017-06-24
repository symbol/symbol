#pragma once
#include "ConnectionSettings.h"
#include "PeerConnectResult.h"
#include "catapult/ionet/PacketHandlers.h"
#include <functional>
#include <memory>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace net { class AsyncTcpServerAcceptContext; }
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace net {

	/// Manages a collection of connections that receive data from external nodes.
	class PacketReaders {
	public:
		using AcceptCallback = std::function<void (PeerConnectResult)>;

	public:
		virtual ~PacketReaders() {}

	public:
		/// Gets the number of active connections (including pending connections).
		virtual size_t numActiveConnections() const = 0;

		/// Gets the number of active readers.
		virtual size_t numActiveReaders() const = 0;

	public:
		/// Accepts a connection represented by \a pAcceptContext and calls \a callback on completion.
		virtual void accept(
				const std::shared_ptr<AsyncTcpServerAcceptContext>& pAcceptContext,
				const AcceptCallback& callback) = 0;

		/// Shutdowns all connections.
		virtual void shutdown() = 0;
	};

	/// Creates a packet readers container for a server with a key pair of \a keyPair using \a pPool and \a handlers
	/// and configured with \a settings.
	std::shared_ptr<PacketReaders> CreatePacketReaders(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const ionet::ServerPacketHandlers& handlers,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings);
}}
