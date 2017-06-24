#pragma once
#include "ConfirmedTimestampedHashesFilter.h"
#include "catapult/ionet/PacketHandlers.h"

namespace catapult { namespace handlers {

	/// Registers a confirm timestamped hashes handler in \a handlers that responds with timestamped hashes
	/// not filtered by the provided filter (\a confirmedTimestampedHashesFilter).
	void RegisterConfirmTimestampedHashesHandler(
			ionet::ServerPacketHandlers& handlers,
			const ConfirmedTimestampedHashesFilter& confirmedTimestampedHashesFilter);
}}
