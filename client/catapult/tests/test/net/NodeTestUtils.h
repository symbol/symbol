#pragma once
#include "catapult/ionet/Node.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Creates a node endpoint referencing the local host with the specified \a port.
	inline ionet::NodeEndpoint CreateLocalHostNodeEndpoint(unsigned short port = Local_Host_Port) {
		return { "127.0.0.1", port };
	}

	/// Creates a node referencing the local host with the specified \a port and public key (\a publicKey).
	inline ionet::Node CreateLocalHostNode(const Key& publicKey, unsigned short port = Local_Host_Port) {
		return { CreateLocalHostNodeEndpoint(port), { publicKey, "" }, model::NetworkIdentifier::Zero };
	}
}}
