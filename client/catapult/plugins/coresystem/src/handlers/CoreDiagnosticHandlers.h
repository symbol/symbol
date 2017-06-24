#pragma once
#include "AccountInfosSupplier.h"
#include "catapult/ionet/PacketHandlers.h"

namespace catapult { namespace handlers {

	/// Registers an account infos handler in \a handlers that responds with account infos
	/// returned by the supplier (\a accountInfosSupplier).
	void RegisterAccountInfosHandler(ionet::ServerPacketHandlers& handlers, const AccountInfosSupplier& accountInfosSupplier);
}}
