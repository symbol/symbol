#pragma once
#include "ConnectionSettings.h"
#include "PeerConnectResult.h"
#include "catapult/functions.h"
#include <memory>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace ionet {
		class Node;
		class PacketSocket;
	}
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace net {

	/// Bootstraps connections to external nodes to which this node connects.
	class ServerConnector {
	public:
		/// A callback that is passed the connect result and the connected socket on success.
		using ConnectCallback = consumer<PeerConnectResult, const std::shared_ptr<ionet::PacketSocket>&>;

	public:
		virtual ~ServerConnector() {}

	public:
		/// Gets the number of active connections.
		virtual size_t numActiveConnections() const = 0;

	public:
		/// Attempts to connect to \a node and calls \a callback on completion.
		virtual void connect(const ionet::Node& node, const ConnectCallback& callback) = 0;

		/// Shutdowns all connections.
		virtual void shutdown() = 0;
	};

	/// Creates a server connector for a server with a key pair of \a keyPair using \a pPool and configured with \a settings.
	std::shared_ptr<ServerConnector> CreateServerConnector(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings);
}}
