#pragma once
#include "catapult/ionet/Node.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace ionet { class PacketIo; } }

namespace catapult { namespace api {

	/// An api for retrieving node information from a remote node.
	class RemoteNodeApi {
	public:
		virtual ~RemoteNodeApi() {}

	public:
		/// Gets information about the node.
		virtual thread::future<ionet::Node> nodeInfo() const = 0;

		/// Gets information about the node's peers.
		virtual thread::future<ionet::NodeSet> peersInfo() const = 0;
	};

	/// Creates a node api for interacting with a remote node with the specified \a io.
	std::unique_ptr<RemoteNodeApi> CreateRemoteNodeApi(ionet::PacketIo& io);
}}
