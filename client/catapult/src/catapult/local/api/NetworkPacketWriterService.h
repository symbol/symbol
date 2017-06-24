#pragma once
#include "catapult/local/Sinks.h"
#include "catapult/utils/NonCopyable.h"

namespace catapult {
	namespace config { class LocalNodeConfiguration; }
	namespace crypto { class KeyPair; }
	namespace net {
		class AsyncTcpServer;
		class PacketWriters;
	}
	namespace thread { class MultiServicePool; }
}

namespace catapult { namespace local { namespace api {

	/// A service for managing api packet writers (used for sending packets to p2p nodes).
	class NetworkPacketWriterService : public utils::NonCopyable {
	public:
		/// Creates a service around \a keyPair and \a config.
		NetworkPacketWriterService(const crypto::KeyPair& keyPair, const config::LocalNodeConfiguration& config);

	public:
		/// Gets the number of active writers.
		size_t numActiveWriters() const;

	public:
		/// Gets the packet writers container.
		net::PacketWriters& packetWriters();

	public:
		/// Gets a transactions sink that broadcasts transactions.
		SharedNewTransactionsSink createNewTransactionsSink();

	public:
		/// Boots the service with \a pool parallelization.
		void boot(thread::MultiServicePool& pool);

	private:
		template<typename TSink>
		TSink createPushEntitySink() const;

	private:
		// state
		const crypto::KeyPair& m_keyPair;
		const config::LocalNodeConfiguration& m_config;

		// services
		std::weak_ptr<net::PacketWriters> m_pWriters;
	};
}}}
