#pragma once
#include "PacketHandlers.h"
#include "SocketOperationCode.h"
#include <memory>

namespace catapult {
	namespace ionet {
		class BatchPacketReader;
		class PacketIo;
	}
}

namespace catapult { namespace ionet {

	/// Reads and consumes packets from a socket.
	class SocketReader {
	public:
		using ReadCallback = std::function<void (SocketOperationCode)>;

	public:
		virtual ~SocketReader() {}

	public:
		/// Reads and consumes one or more packets and calls \a callback on completion.
		virtual void read(const ReadCallback& callback) = 0;
	};

	/// Creates a socket packet reader around \a pReader, \a pWriter, and \a handlers.
	std::unique_ptr<SocketReader> CreateSocketReader(
			const std::shared_ptr<BatchPacketReader>& pReader,
			const std::shared_ptr<PacketIo>& pWriter,
			const ServerPacketHandlers& handlers);
}}
