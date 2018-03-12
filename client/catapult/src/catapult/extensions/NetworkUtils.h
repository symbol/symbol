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
