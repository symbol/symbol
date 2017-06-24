#include "NetworkUtils.h"

namespace catapult { namespace local {

	net::ConnectionSettings GetConnectionSettings(const config::LocalNodeConfiguration& config) {
		net::ConnectionSettings settings;
		settings.NetworkIdentifier = config.BlockChain.Network.Identifier;
		settings.Timeout = config.Node.ConnectTimeout;
		settings.SocketWorkingBufferSize = config.Node.SocketWorkingBufferSize;
		settings.MaxPacketDataSize = config.Node.MaxPacketDataSize;
		return settings;
	}
}}
