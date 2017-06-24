#pragma once
#include "PacketIo.h"

namespace catapult { namespace ionet {

	/// An read-optimized interface for reading packets.
	class BatchPacketReader {
	public:
		virtual ~BatchPacketReader() {}

	public:
		/// Reads and consumes all ready packets and calls \a callback on completion.
		/// On success, the read packets are passed to \a callback.
		/// \note A batch read operation will always result in the callback being called
		///       with a non-success result, which indicates the completion of the
		///       batch operation.
		virtual void readMultiple(const PacketIo::ReadCallback& callback) = 0;
	};
}}
