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

#include "NetworkUtils.h"

namespace catapult { namespace extensions {

	net::ConnectionSettings GetConnectionSettings(const config::CatapultConfiguration& config) {
		net::ConnectionSettings settings;
		settings.NetworkIdentifier = config.BlockChain.Network.Identifier;
		settings.Timeout = config.Node.ConnectTimeout;
		settings.SocketWorkingBufferSize = config.Node.SocketWorkingBufferSize;
		settings.SocketWorkingBufferSensitivity = config.Node.SocketWorkingBufferSensitivity;
		settings.MaxPacketDataSize = config.Node.MaxPacketDataSize;

		settings.OutgoingSecurityMode = config.Node.OutgoingSecurityMode;
		settings.IncomingSecurityModes = config.Node.IncomingSecurityModes;
		return settings;
	}

	void UpdateAsyncTcpServerSettings(net::AsyncTcpServerSettings& settings, const config::CatapultConfiguration& config) {
		settings.PacketSocketOptions = GetConnectionSettings(config).toSocketOptions();
		settings.AllowAddressReuse = config.Node.EnableAddressReuse;

		const auto& connectionsConfig = config.Node.IncomingConnections;
		settings.MaxActiveConnections = connectionsConfig.MaxConnections;
		settings.MaxPendingConnections = connectionsConfig.BacklogSize;
	}

	uint32_t GetMaxIncomingConnectionsPerIdentity(ionet::NodeRoles roles) {
		// always allow an additional connection per identity in order to not reject ephemeral connections from partners
		auto count = 1u;

		// only count roles that require (separate) incoming connections, not all set roles
		for (auto role : { ionet::NodeRoles::Peer, ionet::NodeRoles::Api })
			count += HasFlag(role, roles) ? 1 : 0;

		return count;
	}
}}
