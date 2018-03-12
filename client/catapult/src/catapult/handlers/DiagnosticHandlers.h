#pragma once
#include "catapult/ionet/PacketHandlers.h"
#include <vector>

namespace catapult {
	namespace ionet { class NodeContainer; }
	namespace utils { class DiagnosticCounter; }
}

namespace catapult { namespace handlers {

	/// Registers a diagnostic counters handler in \a handlers that responds with the current values of \a counters.
	void RegisterDiagnosticCountersHandler(ionet::ServerPacketHandlers& handlers, const std::vector<utils::DiagnosticCounter>& counters);

	/// Registers a diagnostic nodes handler in \a handlers that responds with info about all (active) partner nodes in \a nodeContainer.
	void RegisterDiagnosticNodesHandler(ionet::ServerPacketHandlers& handlers, const ionet::NodeContainer& nodeContainer);
}}
