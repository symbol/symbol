#pragma once
#include "catapult/ionet/IoTypes.h"
#include "catapult/ionet/PacketIo.h"
#include "catapult/types.h"
#include <memory>

namespace boost { namespace asio { class io_service; } }
namespace catapult { namespace net { class PacketWriters; } }

namespace catapult { namespace test {

	/// Creates a connection to localhost on \a port configured with server public key \a serverPublicKey
	/// using \a service.
	std::shared_ptr<ionet::PacketIo> ConnectToLocalHost(
			boost::asio::io_service& service,
			unsigned short port,
			const Key& serverPublicKey);

	/// Creates a connection to localhost configured with server public key \a serverPublicKey
	/// using \a packetWriters.
	void ConnectToLocalHost(net::PacketWriters& packetWriters, const Key& serverPublicKey);

	/// Starts an async read on \a io that fills \a buffer.
	void AsyncReadIntoBuffer(ionet::PacketIo& io, ionet::ByteBuffer& buffer);

	/// Creates a default packet writers holder.
	class DefaultPacketWritersHolder {
	public:
		/// Creates a holder.
		DefaultPacketWritersHolder();

		/// Destroys the holder.
		~DefaultPacketWritersHolder();

	public:
		/// Gets the packet writers.
		net::PacketWriters& get();

	private:
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}}
