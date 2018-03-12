#pragma once
#include "Node.h"
#include <memory>

namespace catapult { namespace ionet {

#pragma pack(push, 1)

	/// Information about a catapult node that is propagated through the network.
	struct NetworkNode {
		/// Size of the node.
		uint32_t Size;

		/// The unique identifier (public key).
		Key IdentityKey;

		/// The port.
		uint16_t Port;

		/// The network identifier.
		model::NetworkIdentifier NetworkIdentifier;

		/// The version.
		NodeVersion Version;

		/// The role(s).
		NodeRoles Roles;

		/// The size of the host in bytes.
		uint8_t HostSize;

		/// The size of the friendly name in bytes.
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

	/// Packs a \a node model into a network node.
	std::unique_ptr<NetworkNode> PackNode(const Node& node);

	/// Unpacks a network node (\a networkNode) into a node model.
	Node UnpackNode(const NetworkNode& networkNode);
}}
