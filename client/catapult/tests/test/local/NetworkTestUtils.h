#pragma once
#include "catapult/ionet/IoTypes.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/types.h"
#include <memory>

namespace boost { namespace asio { class io_service; } }

namespace catapult { namespace net { class PacketWriters; } }

namespace catapult { namespace test {

	/// Creates a connection to localhost on \a port configured with server public key \a serverPublicKey using \a service.
	std::shared_ptr<ionet::PacketSocket> ConnectToLocalHost(
			boost::asio::io_service& service,
			unsigned short port,
			const Key& serverPublicKey);

	/// Creates a connection to localhost configured with server public key \a serverPublicKey
	/// using \a packetWriters.
	void ConnectToLocalHost(net::PacketWriters& packetWriters, const Key& serverPublicKey);

	/// Starts an async read on \a io that fills \a buffer using \a service.
	void AsyncReadIntoBuffer(boost::asio::io_service& service, ionet::PacketSocket& io, ionet::ByteBuffer& buffer);
}}
