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
#include "Results.h"
#include "catapult/ionet/ReadRateMonitorSocketDecorator.h"
#include "catapult/net/ConnectionContainer.h"
#include "catapult/net/PeerConnectResult.h"
#include "catapult/subscribers/NodeSubscriber.h"

namespace catapult { namespace extensions {

	// region GetRateMonitorSettings

	ionet::RateMonitorSettings GetRateMonitorSettings(const config::NodeConfiguration::BanningSubConfiguration& banConfig) {
		ionet::RateMonitorSettings rateMonitorSettings;
		rateMonitorSettings.NumBuckets = banConfig.NumReadRateMonitoringBuckets;
		rateMonitorSettings.BucketDuration = banConfig.ReadRateMonitoringBucketDuration;
		rateMonitorSettings.MaxTotalSize = banConfig.MaxReadRateMonitoringTotalSize;
		return rateMonitorSettings;
	}

	// endregion

	// region GetConnectionSettings / UpdateAsyncTcpServerSettings

	net::ConnectionSettings GetConnectionSettings(const config::CatapultConfiguration& config) {
		net::ConnectionSettings settings;
		settings.NetworkIdentifier = config.BlockChain.Network.Identifier;
		settings.NodeIdentityEqualityStrategy = config.BlockChain.Network.NodeEqualityStrategy;

		settings.Timeout = config.Node.ConnectTimeout;
		settings.SocketWorkingBufferSize = config.Node.SocketWorkingBufferSize;
		settings.SocketWorkingBufferSensitivity = config.Node.SocketWorkingBufferSensitivity;
		settings.MaxPacketDataSize = config.Node.MaxPacketDataSize;

		settings.SslOptions.ContextSupplier = ionet::CreateSslContextSupplier(config.User.CertificateDirectory);
		settings.SslOptions.VerifyCallbackSupplier = ionet::CreateSslVerifyCallbackSupplier();
		return settings;
	}

	void UpdateAsyncTcpServerSettings(net::AsyncTcpServerSettings& settings, const config::CatapultConfiguration& config) {
		settings.PacketSocketOptions = GetConnectionSettings(config).toSocketOptions();
		settings.AllowAddressReuse = config.Node.EnableAddressReuse;

		const auto& connectionsConfig = config.Node.IncomingConnections;
		settings.MaxActiveConnections = connectionsConfig.MaxConnections;
		settings.MaxPendingConnections = connectionsConfig.BacklogSize;
	}

	// endregion

	// region BootServer

	namespace {
		struct BootServerState {
		public:
			BootServerState(
					unsigned short port,
					ionet::ServiceIdentifier serviceId,
					subscribers::NodeSubscriber& nodeSubscriber,
					net::AcceptedConnectionContainer& acceptor)
					: Port(port)
					, ServiceId(serviceId)
					, NodeSubscriber(nodeSubscriber)
					, Acceptor(acceptor)
			{}

		public:
			unsigned short Port;
			ionet::ServiceIdentifier ServiceId;
			subscribers::NodeSubscriber& NodeSubscriber;
			net::AcceptedConnectionContainer& Acceptor;
		};
	}

	std::shared_ptr<net::AsyncTcpServer> BootServer(
			thread::MultiServicePool::ServiceGroup& serviceGroup,
			unsigned short port,
			ionet::ServiceIdentifier serviceId,
			const config::CatapultConfiguration& config,
			const supplier<Timestamp>& timeSupplier,
			subscribers::NodeSubscriber& nodeSubscriber,
			net::AcceptedConnectionContainer& acceptor) {
		auto endpoint = config.Node.ListenInterface.empty()
				? boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)
				: boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(config.Node.ListenInterface), port);
		BootServerState bootServerState(port, serviceId, nodeSubscriber, acceptor);
		auto rateMonitorSettings = GetRateMonitorSettings(config.Node.Banning);

		auto serverSettings = net::AsyncTcpServerSettings([bootServerState, rateMonitorSettings, timeSupplier](const auto& socketInfo) {
			auto pCurrentNodeIdentity = std::make_shared<model::NodeIdentity>();
			pCurrentNodeIdentity->Host = socketInfo.host();

			auto rateExceededHandler = [bootServerState, pCurrentNodeIdentity]() {
				bootServerState.NodeSubscriber.notifyBan(
						*pCurrentNodeIdentity,
						utils::to_underlying_type(Failure_Extension_Read_Rate_Limit_Exceeded));
				bootServerState.Acceptor.closeOne(*pCurrentNodeIdentity);
			};
			ionet::PacketSocketInfo decoratedSocketInfo(
					socketInfo.host(),
					socketInfo.publicKey(),
					ionet::AddReadRateMonitor(socketInfo.socket(), rateMonitorSettings, timeSupplier, rateExceededHandler));

			bootServerState.Acceptor.accept(decoratedSocketInfo, [bootServerState, pCurrentNodeIdentity](const auto& connectResult) {
				// on accept failure, only host (not identity key) is known
				CATAPULT_LOG(info)
						<< "accept result to local node port " << bootServerState.Port
						<< " from " << pCurrentNodeIdentity->Host << ": " << connectResult.Code;

				if (net::PeerConnectCode::Accepted != connectResult.Code)
					return false;

				*pCurrentNodeIdentity = connectResult.Identity;
				if (bootServerState.NodeSubscriber.notifyIncomingNode(connectResult.Identity, bootServerState.ServiceId))
					return true;

				bootServerState.Acceptor.closeOne(connectResult.Identity);
				return false;
			});
		});

		UpdateAsyncTcpServerSettings(serverSettings, config);
		return serviceGroup.pushService(net::CreateAsyncTcpServer, endpoint, serverSettings);
	}

	// endregion
}}
