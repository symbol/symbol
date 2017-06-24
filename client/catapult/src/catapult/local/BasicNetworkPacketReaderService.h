#pragma once
#include "LocalNodeStateRef.h"
#include "catapult/utils/NonCopyable.h"
#include <memory>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace ionet { class ServerPacketHandlers; }
	namespace net {
		class AsyncTcpServer;
		class PacketReaders;
	}
	namespace thread { class MultiServicePool; }
}

namespace catapult { namespace local {

	/// A basic service for handling client requests (used for reading packets from tcp).
	class BasicNetworkPacketReaderService : public utils::NonCopyable {
	protected:
		/// Creates a service around \a keyPair and \a stateRef.
		BasicNetworkPacketReaderService(const crypto::KeyPair& keyPair, const LocalNodeStateConstRef& stateRef);

		/// Destroys the service.
		virtual ~BasicNetworkPacketReaderService() {}

	public:
		/// Gets the number of active readers.
		size_t numActiveReaders() const;

	public:
		/// Boots the service with \a pool parallelization.
		void boot(thread::MultiServicePool& pool);

	private:
		virtual void registerHandlers(ionet::ServerPacketHandlers& serverPacketHandlers, const LocalNodeStateConstRef& stateRef) = 0;

	private:
		// state
		const crypto::KeyPair& m_keyPair;
		LocalNodeStateConstRef m_stateRef;

		// services
		std::weak_ptr<net::PacketReaders> m_pReaders;
		std::weak_ptr<net::AsyncTcpServer> m_pPeerServer;
	};
}}
