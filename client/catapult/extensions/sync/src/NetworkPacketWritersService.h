#pragma once
#include "catapult/extensions/ServiceRegistrar.h"
#include "catapult/net/PacketWriters.h"

namespace catapult { namespace sync {

	/// Gets the packet writers service from \a locator.
	std::shared_ptr<net::PacketWriters> GetPacketWriters(const extensions::ServiceLocator& locator);

	/// Creates a registrar for a packet writers service.
	/// \note This service is responsible for sending data to peers.
	DECLARE_SERVICE_REGISTRAR(NetworkPacketWriters)();
}}
