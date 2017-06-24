#pragma once
#include "NetworkHeight.h"
#include "catapult/chain/ScheduledHarvesterTask.h"
#include "catapult/handlers/DiagnosticHandlers.h"

namespace catapult {
	namespace io { class BlockStorageCache; }
	namespace net { class PacketIoPicker; }
}

namespace catapult { namespace local { namespace p2p {

	using ChainSyncedPredicate = std::function<bool ()>;

	/// Creates a retriever that uses \a packetIoPicker to retrieve the chain heights of peers.
	RemoteChainHeightsRetriever CreateRemoteChainHeightsRetriever(net::PacketIoPicker& packetIoPicker);

	/// Predicate that uses the local \a storage and a network chain height supplier (\a networkChainHeightSupplier)
	/// to determine if the local node's chain is synchronized with the rest of the network.
	ChainSyncedPredicate CreateChainSyncedPredicate(
			const io::BlockStorageCache& storage,
			const NetworkChainHeightSupplier& networkChainHeightSupplier);
}}}
