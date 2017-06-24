#pragma once
#include <stddef.h>

namespace catapult { namespace ionet {

	/// Packet socket options.
	struct PacketSocketOptions {
		/// The working buffer size.
		size_t WorkingBufferSize;

		/// The maximum packet data size.
		size_t MaxPacketDataSize;
	};
}}
