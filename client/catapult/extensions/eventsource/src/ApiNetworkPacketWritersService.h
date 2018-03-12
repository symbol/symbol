#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace eventsource {

	/// Creates a registrar for an api packet writers service.
	/// \note This service is responsible for sending data to api subscribers.
	DECLARE_SERVICE_REGISTRAR(ApiNetworkPacketWriters)();
}}
