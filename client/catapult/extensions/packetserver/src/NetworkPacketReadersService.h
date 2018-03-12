#pragma once
#include "catapult/extensions/ServiceRegistrar.h"
#include "catapult/net/PacketReaders.h"

namespace catapult { namespace packetserver {

	/// Creates a registrar for a packet readers service.
	/// \note This service is responsible for receiving data from peers.
	DECLARE_SERVICE_REGISTRAR(NetworkPacketReaders)();
}}
