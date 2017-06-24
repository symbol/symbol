#pragma once
#include "catapult/ionet/PacketHandlers.h"
#include <vector>

namespace catapult { namespace utils { class DiagnosticCounter; } }

namespace catapult { namespace handlers {

	/// Registers a diagnostic counters handler in \a handlers that responds with the current values of \a counters.
	void RegisterDiagnosticCountersHandler(
			ionet::ServerPacketHandlers& handlers,
			const std::vector<utils::DiagnosticCounter>& counters);
}}
