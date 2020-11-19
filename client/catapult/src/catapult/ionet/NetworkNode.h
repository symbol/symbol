/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "Node.h"
#include <memory>

namespace catapult { namespace ionet {

#pragma pack(push, 1)

	/// Information about a catapult node that is propagated through the network.
	struct NetworkNode {
		/// Size of the node.
		uint32_t Size;

		/// Version.
		NodeVersion Version;

		/// Unique node identifier (public key).
		Key IdentityKey;

		/// Network generation hash seed.
		GenerationHashSeed NetworkGenerationHashSeed;

		/// Role(s).
		NodeRoles Roles;

		/// Port.
		uint16_t Port;

		/// Network identifier.
		model::NetworkIdentifier NetworkIdentifier;

		/// Size of the host in bytes.
		uint8_t HostSize;

		/// Size of the friendly name in bytes.
		uint8_t FriendlyNameSize;

		// followed by host if HostSize != 0
		// followed by friendly name if FriendlyNameSize != 0

	public:
		/// Calculates the real size of \a node.
		static constexpr uint64_t CalculateRealSize(const NetworkNode& node) noexcept {
			return sizeof(NetworkNode) + node.HostSize + node.FriendlyNameSize;
		}
	};

#pragma pack(pop)

	/// Packs \a node model into a network node.
	std::unique_ptr<NetworkNode> PackNode(const Node& node);

	/// Unpacks a network node (\a networkNode) into a node model.
	Node UnpackNode(const NetworkNode& networkNode);
}}
