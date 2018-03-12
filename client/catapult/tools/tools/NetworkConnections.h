#pragma once
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/NodePacketIoPair.h"
#include "catapult/thread/Future.h"

namespace catapult {
	namespace net { class PacketWriters; }
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace tools {

	/// Class that holds connections to network peers.
	class NetworkConnections {
	public:
		/// Creates a network connections object around \a nodes.
		explicit NetworkConnections(const std::vector<ionet::Node>& nodes);

		/// Move constructor.
		NetworkConnections(NetworkConnections&& rhs) = default;

		/// Destroys the connections.
		~NetworkConnections();

	public:
		/// Gets the number of active connections (including pending connections).
		size_t numActiveConnections() const;

		/// Gets the number of active writers.
		size_t numActiveWriters() const;

		/// Gets the number of available writers.
		/// \note There will be fewer available writers than active writers when some writers are checked out.
		size_t numAvailableWriters() const;

	public:
		/// Establishes connections to all peers in the network.
		thread::future<bool> connectAll();

		/// Picks one network connection.
		ionet::NodePacketIoPair pickOne() const;

	private:
		void shutdown();

	private:
		std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
		crypto::KeyPair m_clientKeyPair;
		const std::vector<ionet::Node> m_nodes;
		std::shared_ptr<net::PacketWriters> m_pPacketWriters;
	};

	/// Gets the current network height using \a connections.
	Height GetHeight(const NetworkConnections& connections);

	/// Waits for the network to produce \a numBlocks blocks using \a connections.
	bool WaitForBlocks(const NetworkConnections& connections, size_t numBlocks);
}}
