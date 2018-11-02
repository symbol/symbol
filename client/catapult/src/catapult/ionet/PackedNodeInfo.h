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
#include "NodeInfo.h"
#include "catapult/model/TrailingVariableDataLayout.h"

namespace catapult { namespace ionet {

#pragma pack(push, 1)

	/// Connection state unique to a node and connection identifier.
	struct PackedConnectionState {
	public:
		/// Connection identifier.
		ServiceIdentifier ServiceId;

		/// Current connection age.
		/// \c 0 if the connection is not active.
		uint32_t Age;

		/// Number of connection attempts.
		uint32_t NumAttempts;

		/// Number of successful connections.
		uint32_t NumSuccesses;

		/// Number of failed connections.
		uint32_t NumFailures;

		/// Number of consecutive failed connections.
		uint32_t NumConsecutiveFailures;

		/// Current ban age.
		/// \c 0 if the connection is not banned.
		uint32_t BanAge;

	public:
		/// Updates values with corresponding values from \a from \a connectionState.
		void Update(const ConnectionState& connectionState) {
			Age = connectionState.Age;
			NumAttempts = connectionState.NumAttempts;
			NumSuccesses = connectionState.NumSuccesses;
			NumFailures = connectionState.NumFailures;
			NumConsecutiveFailures = connectionState.NumConsecutiveFailures;
			BanAge = connectionState.BanAge;
		}
	};

	/// Information about a node and its interactions.
	struct PackedNodeInfo : public model::TrailingVariableDataLayout<PackedNodeInfo, PackedConnectionState> {
	public:
		/// Node unique identifier.
		Key IdentityKey;

		/// Node source.
		NodeSource Source;

		/// Number of connection states.
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
