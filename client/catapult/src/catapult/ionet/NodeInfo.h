/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

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
	/* Local node. */ \
	ENUM_VALUE(Local)

#define ENUM_VALUE(LABEL) LABEL,
	/// Source of a node.
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
		/// Current connection age.
		/// \c 0 if the connection is not active.
		uint32_t Age;

		/// Number of connection attempts.
		uint32_t NumAttempts;

		/// Number of successful connections.
		uint32_t NumSuccesses;

		/// Number of failed connections.
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
