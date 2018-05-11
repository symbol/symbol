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
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/net/AsyncTcpServer.h"
#include "catapult/net/ConnectionSettings.h"
#include "catapult/net/PeerConnectResult.h"
#include "catapult/thread/MultiServicePool.h"

namespace catapult { namespace extensions {

	/// Extracts connection settings from \a config.
	net::ConnectionSettings GetConnectionSettings(const config::LocalNodeConfiguration& config);

	/// Updates \a settings with values in \a config.
	void UpdateAsyncTcpServerSettings(net::AsyncTcpServerSettings& settings, const config::LocalNodeConfiguration& config);

	/// Gets the maximum number of incoming connections per identity as specified by \a roles.
	uint32_t GetMaxIncomingConnectionsPerIdentity(ionet::NodeRoles roles);

	/// Boots a tcp server with \a serviceGroup on localhost \a port with connection \a config and \a acceptor.
	template<typename TAcceptor>
	std::shared_ptr<net::AsyncTcpServer> BootServer(
			thread::MultiServicePool::ServiceGroup& serviceGroup,
			unsigned short port,
			const config::LocalNodeConfiguration& config,
			TAcceptor acceptor) {
		auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port);
		auto settings = net::AsyncTcpServerSettings([acceptor, port](const auto& socketInfo) {
			acceptor(socketInfo, [port](auto result) {
				CATAPULT_LOG(info) << "accept result to local node port " << port << ": " << result;
			});
		});

		UpdateAsyncTcpServerSettings(settings, config);
		return serviceGroup.pushService(net::CreateAsyncTcpServer, endpoint, settings);
	}
}}
