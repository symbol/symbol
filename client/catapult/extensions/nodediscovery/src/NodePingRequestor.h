#pragma once
#include "catapult/ionet/Node.h"
#include "catapult/functions.h"
#include <memory>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace net { struct ConnectionSettings; }
	namespace thread { class IoServiceThreadPool; }
}

namespace catapult { namespace nodediscovery {

#define NODE_PING_RESULT_LIST \
	/* The connection to the remote node failed. */ \
	ENUM_VALUE(Failure_Connection) \
	/* The interaction with the remote node failed. */ \
	ENUM_VALUE(Failure_Interaction) \
	/* The remote node is incompatible with the network. */ \
	ENUM_VALUE(Failure_Incompatible) \
	/* The interaction with the remote node timed out. */ \
	ENUM_VALUE(Failure_Timeout) \
	/* The ping operation succeeded. */ \
	ENUM_VALUE(Success)

#define ENUM_VALUE(LABEL) LABEL,
	/// Enumeration of possible results of a ping operation.
	enum class NodePingResult {
		NODE_PING_RESULT_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, NodePingResult value);

	/// A service for requesting ping information from remote nodes.
	class NodePingRequestor {
	public:
		/// A callback that is passed the ping result and the remote node information on success.
		using PingCallback = consumer<NodePingResult, const ionet::Node&>;

	public:
		virtual ~NodePingRequestor() {}

	public:
		/// Gets the number of active connections.
		virtual size_t numActiveConnections() const = 0;

		/// Gets the number of total ping requests.
		virtual size_t numTotalPingRequests() const = 0;

		/// Gets the number of successful ping requests.
		virtual size_t numSuccessfulPingRequests() const = 0;

	public:
		/// Requests ping information from \a node and calls \a callback on completion.
		virtual void requestPing(const ionet::Node& node, const PingCallback& callback) = 0;

		/// Shutdowns all connections.
		virtual void shutdown() = 0;
	};

	/// Creates a node ping requestor for a server with a key pair of \a keyPair and a network identified by \a networkIdentifier
	/// using \a pPool and configured with \a settings.
	std::shared_ptr<NodePingRequestor> CreateNodePingRequestor(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const crypto::KeyPair& keyPair,
			const net::ConnectionSettings& settings,
			model::NetworkIdentifier networkIdentifier);
}}
