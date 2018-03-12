#pragma once
#include "PacketType.h"

namespace catapult { namespace ionet {

#pragma pack(push, 1)

	/// A packet header that all transferable information is expected to have.
	struct PacketHeader {
		/// The size of the packet.
		uint32_t Size;

		/// The type of the packet.
		PacketType Type;
	};

#pragma pack(pop)
}}
