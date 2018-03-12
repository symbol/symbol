#include "NetworkUtils.h"

namespace catapult { namespace extensions {

	net::ConnectionSettings GetConnectionSettings(const config::LocalNodeConfiguration& config) {
		net::ConnectionSettings settings;
		settings.NetworkIdentifier = config.BlockChain.Network.Identifier;
		settings.Timeout = config.Node.ConnectTimeout;
		settings.SocketWorkingBufferSize = config.Node.SocketWorkingBufferSize;
		settings.SocketWorkingBufferSensitivity = config.Node.SocketWorkingBufferSensitivity;
		settings.MaxPacketDataSize = config.Node.MaxPacketDataSize;
		return settings;
	}

	void UpdateAsyncTcpServerSettings(net::AsyncTcpServerSettings& settings, const config::LocalNodeConfiguration& config) {
		settings.PacketSocketOptions = GetConnectionSettings(config).toSocketOptions();
		settings.AllowAddressReuse = config.Node.ShouldAllowAddressReuse;

		const auto& connectionsConfig = config.Node.IncomingConnections;
		settings.MaxActiveConnections = connectionsConfig.MaxConnections;
		settings.MaxPendingConnections = connectionsConfig.BacklogSize;
	}

	uint32_t GetMaxIncomingConnectionsPerIdentity(ionet::NodeRoles roles) {
		// only count roles that require (separate) incoming connections, not all set roles
		auto count = 0u;
		for (auto role : { ionet::NodeRoles::Peer, ionet::NodeRoles::Api })
			count += HasFlag(role, roles) ? 1 : 0;

		return count;
	}
}}
