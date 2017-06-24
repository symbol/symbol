#pragma once
#include "ConnectionSettings.h"
#include "PeerConnectResult.h"
#include <functional>
#include <memory>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace net { class AsyncTcpServerAcceptContext; }
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace net {

	/// Bootstraps connections from external nodes that connect to this node.
	class ClientConnector {
	public:
		/// A callback that is passed the accept result and the client public key (on success).
		using AcceptCallback = std::function<void (PeerConnectResult, const Key&)>;

	public:
		virtual ~ClientConnector() {}

	public:
		/// Gets the number of active connections.
		virtual size_t numActiveConnections() const = 0;

	public:
		/// Accepts a connection represented by \a pAcceptContext and calls \a callback on completion.
		virtual void accept(
				const std::shared_ptr<AsyncTcpServerAcceptContext>& pAcceptContext,
				const AcceptCallback& callback) = 0;

		/// Shutdowns all connections.
		virtual void shutdown() = 0;
	};

	/// Creates a client connector for a server with a key pair of \a keyPair using \a pPool and configured with
	/// \a settings.
	std::shared_ptr<ClientConnector> CreateClientConnector(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings);
}}
