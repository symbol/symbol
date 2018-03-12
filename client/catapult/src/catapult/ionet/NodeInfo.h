#pragma once
#include "catapult/utils/Hashers.h"
#include "catapult/types.h"
#include <unordered_set>
#include <vector>

namespace catapult { namespace ionet {

#define NODE_SOURCE_LIST \
	/* Forwarded from a peer node. */ \
	ENUM_VALUE(Dynamic) \
	\
	/* Loaded from a local peers file. */ \
	ENUM_VALUE(Static) \
	\
	/* The local node. */ \
	ENUM_VALUE(Local)

#define ENUM_VALUE(LABEL) LABEL,
	/// The source of a node.
	/// \note Higher value sources correspond to higher levels of trust.
	enum class NodeSource : uint32_t {
		NODE_SOURCE_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, NodeSource value);

	struct ServiceIdentifier_tag {};
	using ServiceIdentifier = utils::BaseValue<uint32_t, ServiceIdentifier_tag>;

	/// Connection state unique to a node and connection identifier.
	struct ConnectionState {
	public:
		/// Creates zeroed state.
		ConnectionState()
				: Age(0)
				, NumAttempts(0)
				, NumSuccesses(0)
				, NumFailures(0)
		{}

	public:
		/// The current connection age.
		/// \c 0 if the connection is not active.
		uint32_t Age;

		/// The number of connection attempts.
		uint32_t NumAttempts;

		/// The number of successful connections.
		uint32_t NumSuccesses;

		/// The number of failed connections.
		uint32_t NumFailures;
	};

	/// Information about a node and its interactions.
	struct NodeInfo {
	public:
		/// A container of service identifiers.
		using ServiceIdentifiers = std::unordered_set<ServiceIdentifier, utils::BaseValueHasher<ServiceIdentifier>>;

	public:
		/// Creates a node info with \a source.
		explicit NodeInfo(NodeSource source);

	public:
		/// Gets the node source.
		NodeSource source() const;

		/// Gets the number of connection states.
		size_t numConnectionStates() const;

		/// Gets the associated service ids.
		ServiceIdentifiers services() const;

		/// Returns true if at least one connection is active.
		bool hasActiveConnection() const;

		/// Gets connection state for the service identified by \a serviceId or \c nullptr if no state exists.
		const ConnectionState* getConnectionState(ServiceIdentifier serviceId) const;

	public:
		/// Sets the node source to \a source.
		void source(NodeSource source);

		/// Gets connection state for the service identified by \a serviceId and creates zeroed state if no state exists.
		ConnectionState& provisionConnectionState(ServiceIdentifier serviceId);

		/// Clears the age of the connection for the service identified by \a serviceId.
		void clearAge(ServiceIdentifier serviceId);

	private:
		NodeSource m_source;
		std::vector<std::pair<ServiceIdentifier, ConnectionState>> m_connectionStates;
	};
}}
