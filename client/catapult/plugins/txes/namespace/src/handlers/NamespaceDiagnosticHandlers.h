#pragma once
#include "MosaicInfosSupplier.h"
#include "NamespaceInfosSupplier.h"
#include "catapult/ionet/PacketHandlers.h"

namespace catapult { namespace handlers {

	/// Registers a mosaic infos handler in \a handlers that responds with mosaic infos
	/// returned by the supplier (\a mosaicInfosSupplier).
	void RegisterMosaicInfosHandler(ionet::ServerPacketHandlers& handlers, const MosaicInfosSupplier& mosaicInfosSupplier);

	/// Registers a namespace infos handler in \a handlers that responds with namespace infos
	/// returned by the supplier (\a namespaceInfosSupplier).
	void RegisterNamespaceInfosHandler(ionet::ServerPacketHandlers& handlers, const NamespaceInfosSupplier& namespaceInfosSupplier);
}}
