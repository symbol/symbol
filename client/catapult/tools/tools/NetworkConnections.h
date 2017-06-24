#pragma once
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/NodePacketIoPair.h"
#include "catapult/net/PacketWriters.h"

namespace catapult {
	namespace thread {
		template<typename T>
		class future;
		class IoServiceThreadPool;
	}
}

namespace catapult { namespace tools {

	/// Class that holds connections to network peers.
	class NetworkConnections {
	public:
		/// Creates a network connections object around \a config.
		explicit NetworkConnections(const config::LocalNodeConfiguration& config);

		/// Move constructor
		NetworkConnections(NetworkConnections&& rhs) = default;

	public:
		~NetworkConnections();

	public:
		/// Gets the number of active connections (including pending connections).
		size_t numActiveConnections() const {
			return m_pPacketWriters->numActiveConnections();
		}

		/// Gets the number of active writers.
		size_t numActiveWriters() const {
			return m_pPacketWriters->numActiveWriters();
		}

		/// Gets the number of available writers.
		/// \note There will be fewer available writers than active writers when some writers are checked out.
		size_t numAvailableWriters() const {
			return m_pPacketWriters->numAvailableWriters();
		}

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
		config::LocalNodeConfiguration m_config;
		std::shared_ptr<net::PacketWriters> m_pPacketWriters;
	};
}}
