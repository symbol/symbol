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
#include "catapult/config/CatapultConfiguration.h"
#include "catapult/ionet/NodeInfo.h"
#include "catapult/ionet/RateMonitor.h"
#include "catapult/net/AsyncTcpServer.h"
#include "catapult/net/ConnectionSettings.h"
#include "catapult/thread/MultiServicePool.h"

namespace catapult {
	namespace net { class AcceptedConnectionContainer; }
	namespace subscribers { class NodeSubscriber; }
}

namespace catapult { namespace extensions {

	/// Gets the rate monitor settings from \a banConfig.
	ionet::RateMonitorSettings GetRateMonitorSettings(const config::NodeConfiguration::BanningSubConfiguration& banConfig);

	/// Extracts connection settings from \a config.
	net::ConnectionSettings GetConnectionSettings(const config::CatapultConfiguration& config);

	/// Updates \a settings with values in \a config.
	void UpdateAsyncTcpServerSettings(net::AsyncTcpServerSettings& settings, const config::CatapultConfiguration& config);

	/// Boots a tcp server with \a serviceGroup on localhost \a port with connection \a config and \a acceptor given \a timeSupplier.
	/// Incoming connections are assumed to be associated with \a serviceId and are added to \a nodeSubscriber.
	std::shared_ptr<net::AsyncTcpServer> BootServer(
			thread::MultiServicePool::ServiceGroup& serviceGroup,
			unsigned short port,
			ionet::ServiceIdentifier serviceId,
			const config::CatapultConfiguration& config,
			const supplier<Timestamp>& timeSupplier,
			subscribers::NodeSubscriber& nodeSubscriber,
			net::AcceptedConnectionContainer& acceptor);
}}
