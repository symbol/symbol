#pragma once
#include "catapult/utils/ArraySet.h"

namespace catapult { namespace net {

	/// Manages a collection of connections.
	class ConnectionContainer {
	public:
		virtual ~ConnectionContainer() {}

	public:
		/// Gets the number of active connections (including pending connections).
		virtual size_t numActiveConnections() const = 0;

		/// Gets the identities of active connections.
		virtual utils::KeySet identities() const = 0;

	public:
		/// Closes any active connections to the node identified by \a identityKey.
		virtual bool closeOne(const Key& identityKey) = 0;
	};
}}
