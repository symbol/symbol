#pragma once
#include "ConnectionContainer.h"
#include "ConnectionSettings.h"
#include "PeerConnectResult.h"
#include "catapult/ionet/PacketHandlers.h"
#include <functional>
#include <memory>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace ionet {
		class AcceptedPacketSocketInfo;
		class PacketIo;
		class PacketSocket;
	}
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace net {

	/// Manages a collection of connections that receive data from external nodes.
	class PacketReaders : public ConnectionContainer {
	public:
		using AcceptCallback = consumer<PeerConnectResult>;

	public:
		/// Gets the number of active readers.
		virtual size_t numActiveReaders() const = 0;

	public:
		/// Accepts a connection represented by \a socketInfo and calls \a callback on completion.
		virtual void accept(const ionet::AcceptedPacketSocketInfo& socketInfo, const AcceptCallback& callback) = 0;

		/// Shutdowns all connections.
		virtual void shutdown() = 0;
	};

	/// Creates a packet readers container for a server with a key pair of \a keyPair using \a pPool and \a handlers,
	/// configured with \a settings and allowing \a maxConnectionsPerIdentity.
	std::shared_ptr<PacketReaders> CreatePacketReaders(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const ionet::ServerPacketHandlers& handlers,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings,
			uint32_t maxConnectionsPerIdentity);
}}
