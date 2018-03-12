#include "NetworkNode.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace ionet {

	namespace {
		uint8_t GetPackedSize(const std::string& str) {
			return static_cast<uint8_t>(std::min<size_t>(str.size(), std::numeric_limits<uint8_t>::max()));
		}
	}

	std::unique_ptr<NetworkNode> PackNode(const Node& node) {
		const auto& endpoint = node.endpoint();
		const auto& metadata = node.metadata();

		auto hostSize = GetPackedSize(endpoint.Host);
		auto friendlyNameSize = GetPackedSize(metadata.Name);
		uint32_t packedNodeSize = sizeof(NetworkNode) + hostSize + friendlyNameSize;
		auto pNetworkNode = utils::MakeUniqueWithSize<NetworkNode>(packedNodeSize);

		pNetworkNode->Size = packedNodeSize;
		pNetworkNode->Port = endpoint.Port;
		pNetworkNode->IdentityKey = node.identityKey();
		pNetworkNode->NetworkIdentifier = metadata.NetworkIdentifier;
		pNetworkNode->Version = metadata.Version;
		pNetworkNode->Roles = metadata.Roles;

		pNetworkNode->HostSize = hostSize;
		pNetworkNode->FriendlyNameSize = friendlyNameSize;

		auto* pNetworkNodeData = reinterpret_cast<uint8_t*>(pNetworkNode.get() + 1);
		memcpy(pNetworkNodeData, endpoint.Host.c_str(), hostSize);
		pNetworkNodeData += hostSize;

		memcpy(pNetworkNodeData, metadata.Name.c_str(), friendlyNameSize);
		return pNetworkNode;
	}

	Node UnpackNode(const NetworkNode& networkNode) {
		const auto* pNetworkNodeData = reinterpret_cast<const char*>(&networkNode + 1);

		auto endpoint = NodeEndpoint();
		endpoint.Port = networkNode.Port;
		endpoint.Host = std::string(pNetworkNodeData, networkNode.HostSize);
		pNetworkNodeData += networkNode.HostSize;

		auto metadata = NodeMetadata();
		metadata.NetworkIdentifier = networkNode.NetworkIdentifier;
		metadata.Name = std::string(pNetworkNodeData, networkNode.FriendlyNameSize);
		metadata.Version = networkNode.Version;
		metadata.Roles = networkNode.Roles;

		return Node(networkNode.IdentityKey, endpoint, metadata);
	}
}}
