#pragma once
#include <stddef.h>

namespace catapult { namespace ionet {

	/// Packet socket options.
	struct PacketSocketOptions {
		/// The initial working buffer size.
		size_t WorkingBufferSize;

		/// The working buffer sensitivity.
		size_t WorkingBufferSensitivity;

		/// The maximum packet data size.
		size_t MaxPacketDataSize;
	};
}}
