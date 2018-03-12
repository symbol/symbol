#pragma once
#include "ConnectionContainer.h"
#include "ConnectionSettings.h"
#include "PacketIoPicker.h"
#include "PeerConnectResult.h"
#include <memory>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace ionet {
		class Node;
		class PacketSocket;
	}
	namespace net { struct Packet; }
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace net {

	/// Manages a collection of connections that send data to external nodes.
	class PacketWriters : public ConnectionContainer, public PacketIoPicker {
	public:
		using ConnectCallback = consumer<PeerConnectResult>;

	public:
		/// Gets the number of active writers.
		virtual size_t numActiveWriters() const = 0;

		/// Gets the number of available writers.
		/// \note There will be fewer available writers than active writers when some writers are checked out.
		virtual size_t numAvailableWriters() const = 0;

	public:
		/// Broadcasts \a payload to all active connections.
		virtual void broadcast(const ionet::PacketPayload& payload) = 0;

	public:
		/// Attempts to connect to \a node and calls \a callback on completion.
		virtual void connect(const ionet::Node& node, const ConnectCallback& callback) = 0;

		/// Accepts a connection represented by \a pPacketSocket and calls \a callback on completion.
		virtual void accept(const std::shared_ptr<ionet::PacketSocket>& pPacketSocket, const ConnectCallback& callback) = 0;

		/// Shutdowns all connections.
		virtual void shutdown() = 0;
	};

	/// Creates a packet writers container for a server with a key pair of \a keyPair using \a pPool and configured with
	/// \a settings.
	std::shared_ptr<PacketWriters> CreatePacketWriters(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings);
}}
