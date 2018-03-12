#pragma once
#include "catapult/handlers/HandlerTypes.h"
#include "catapult/ionet/PacketHandlers.h"
#include "catapult/model/Cosignature.h"

namespace catapult { namespace handlers {

	/// Prototype for a function that processes a range of detached cosignatures.
	using CosignatureRangeHandler = RangeHandler<model::DetachedCosignature>;

	/// Registers a push cosignatures handler in \a handlers that forwards cosignatures to \a cosignatureRangeHandler.
	void RegisterPushCosignaturesHandler(ionet::ServerPacketHandlers& handlers, const CosignatureRangeHandler& cosignatureRangeHandler);
}}
