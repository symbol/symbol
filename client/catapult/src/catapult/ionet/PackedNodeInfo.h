#pragma once
#include "NodeInfo.h"
#include "catapult/model/TrailingVariableDataLayout.h"

namespace catapult { namespace ionet {

#pragma pack(push, 1)

	/// Connection state unique to a node and connection identifier.
	struct PackedConnectionState {
		/// The connection identifier.
		ionet::ServiceIdentifier ServiceId;

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
	struct PackedNodeInfo : public model::TrailingVariableDataLayout<PackedNodeInfo, PackedConnectionState> {
	public:
		/// The node unique identifier.
		Key IdentityKey;

		/// The node source.
		NodeSource Source;

		/// The number of connection states.
		uint8_t ConnectionStatesCount;

		// followed by connection states if ConnectionStatesCount != 0

	public:
		/// Returns a const pointer to the first connection state contained in this node info.
		const PackedConnectionState* ConnectionStatesPtr() const {
			return ConnectionStatesCount ? ToTypedPointer(PayloadStart(*this)) : nullptr;
		}

		/// Returns a pointer to the first connection state contained in this node info.
		PackedConnectionState* ConnectionStatesPtr() {
			return ConnectionStatesCount ? ToTypedPointer(PayloadStart(*this)) : nullptr;
		}

	public:
		/// Calculates the real size of \a nodeInfo.
		static constexpr uint64_t CalculateRealSize(const PackedNodeInfo& nodeInfo) noexcept {
			return sizeof(PackedNodeInfo) + nodeInfo.ConnectionStatesCount * sizeof(PackedConnectionState);
		}
	};

#pragma pack(pop)
}}
