#pragma once
#include "IoTypes.h"

namespace catapult { namespace ionet { class PacketIo; } }

namespace catapult { namespace ionet {

	/// Adds buffering to \a pIo using \a strand for synchronization.
	std::shared_ptr<PacketIo> CreateBufferedPacketIo(const std::shared_ptr<PacketIo>& pIo, boost::asio::strand& strand);
}}
