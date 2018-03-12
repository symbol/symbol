#pragma once
#include "Packet.h"
#include "PacketPayload.h"
#include "SocketOperationCode.h"
#include "catapult/functions.h"
#include <memory>

namespace catapult { namespace ionet {

	/// An interface for reading and writing packets.
	class PacketIo {
	public:
		using ReadCallback = consumer<SocketOperationCode, const Packet*>;
		using WriteCallback = consumer<SocketOperationCode>;

	public:
		virtual ~PacketIo() {}

	public:
		/// Reads and consumes the next packet and calls \a callback on completion.
		/// On success, the read packet is passed to \a callback.
		virtual void read(const ReadCallback& callback) = 0;

		/// Writes \a payload and calls \a callback on completion.
		virtual void write(const PacketPayload& payload, const WriteCallback& callback) = 0;
	};
}}
