#pragma once
#include "catapult/crypto/KeyPair.h"
#include "catapult/thread/Future.h"
#include "catapult/types.h"

namespace catapult {
	namespace ionet {
		class Node;
		class PacketIo;
	}
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace tools {

	/// Future that returns a packet io shared pointer.
	using PacketIoFuture = thread::future<std::shared_ptr<ionet::PacketIo>>;

	/// Connects to localhost as a client with \a clientKeyPair using \a pPool.
	/// Localhost is expected to have identity \a serverPublicKey.
	PacketIoFuture ConnectToLocalNode(
			const crypto::KeyPair& clientKeyPair,
			const Key& serverPublicKey,
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool);

	/// Connects to \a node as a client with \a clientKeyPair using \a pPool.
	PacketIoFuture ConnectToNode(
			const crypto::KeyPair& clientKeyPair,
			const ionet::Node& node,
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool);

	/// Helper class for connecting to multiple nodes.
	class MultiNodeConnector {
	public:
		/// Creates a connector.
		MultiNodeConnector();

		/// Destroys the connector.
		~MultiNodeConnector();

	public:
		/// Gets the underlying pool used by the connector.
		thread::IoServiceThreadPool& pool();

	public:
		/// Connects to \a node.
		PacketIoFuture connect(const ionet::Node& node);

	private:
		crypto::KeyPair m_clientKeyPair;
		std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
	};
}}
