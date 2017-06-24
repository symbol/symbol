#pragma once
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/net/AsyncTcpServer.h"
#include "catapult/net/ConnectionSettings.h"
#include "catapult/net/PeerConnectResult.h"
#include "catapult/thread/MultiServicePool.h"

namespace catapult { namespace local {

	/// Extracts connection settings from \a config.
	net::ConnectionSettings GetConnectionSettings(const config::LocalNodeConfiguration& config);

	/// Boots a tcp server with \a serviceGroup on localhost \a port with connection \a config and \a acceptor.
	template<typename TAcceptor>
	std::shared_ptr<net::AsyncTcpServer> BootServer(
			thread::MultiServicePool::ServiceGroup& serviceGroup,
			unsigned short port,
			const config::LocalNodeConfiguration& config,
			TAcceptor& acceptor) {
		auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port);
		auto settings = net::AsyncTcpServerSettings([&acceptor, port](const auto& pAcceptContext) {
			acceptor.accept(pAcceptContext, [port](auto result) {
				CATAPULT_LOG(info) << "accept result to local node port " << port << ": " << result;
			});
		});

		settings.PacketSocketOptions = GetConnectionSettings(config).toSocketOptions();
		settings.AllowAddressReuse = config.Node.ShouldAllowAddressReuse;
		return serviceGroup.pushService(net::CreateAsyncTcpServer, endpoint, settings);
	}
}}
