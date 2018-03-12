#pragma once
#include "catapult/ionet/Node.h"
#include "catapult/net/PacketIoPickerContainer.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace nodediscovery {

	/// Creates a batch peers requestor.
	class BatchPeersRequestor {
	private:
		using NodesConsumer = consumer<const ionet::NodeSet&>;

	public:
		/// Creates a requestor around \a packetIoPickers, which is used to find partners, and forwards found nodes to \a nodesConsumer.
		explicit BatchPeersRequestor(const net::PacketIoPickerContainer& packetIoPickers, const NodesConsumer& nodesConsumer);

	public:
		/// Finds and forwards peers of peers within the specified \a timeout.
		thread::future<bool> findPeersOfPeers(const utils::TimeSpan& timeout) const;

	private:
		net::PacketIoPickerContainer m_packetIoPickers; // held by value because packet io pickers is tied to ServiceState
		NodesConsumer m_nodesConsumer;
	};
}}
