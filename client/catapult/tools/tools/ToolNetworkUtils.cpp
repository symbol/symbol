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

#include "ToolNetworkUtils.h"
#include "ToolKeys.h"
#include "ToolThreadUtils.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/ServerConnector.h"
#include "catapult/thread/IoThreadPool.h"

namespace catapult { namespace tools {

	net::ConnectionSettings CreateToolConnectionSettings(const std::string& certificateDirectory) {
		auto settings = net::ConnectionSettings();
		settings.NetworkIdentifier = model::NetworkIdentifier::Private_Test;
		settings.AllowOutgoingSelfConnections = true;

		settings.SslOptions.ContextSupplier = ionet::CreateSslContextSupplier(certificateDirectory);
		settings.SslOptions.VerifyCallbackSupplier = ionet::CreateSslVerifyCallbackSupplier();
		return settings;
	}

	namespace {
		auto MakePeerConnectException(const ionet::Node& node, net::PeerConnectCode connectCode) {
			std::ostringstream out;
			out << "connecting to " << node << " failed with " << connectCode;
			return std::make_exception_ptr(catapult_runtime_error(out.str().c_str()));
		}

		PacketSocketInfoFuture ConnectToNodeEx(
				const net::ConnectionSettings& connectionSettings,
				const ionet::Node& node,
				thread::IoThreadPool& pool) {
			auto pPromise = std::make_shared<thread::promise<ionet::PacketSocketInfo>>();

			// it is ok to pass empty Key() because key is only used to disallow connections to self
			// and AllowOutgoingSelfConnections is set
			auto pConnector = net::CreateServerConnector(pool, Key(), connectionSettings, "tool");
			pConnector->connect(node, [node, pPromise](auto connectResult, const auto& socketInfo) {
				switch (connectResult) {
				case net::PeerConnectCode::Accepted:
					return pPromise->set_value(ionet::PacketSocketInfo(socketInfo));

				default:
					CATAPULT_LOG(fatal) << "error occurred when trying to connect to node: " << connectResult;
					return pPromise->set_exception(MakePeerConnectException(node, connectResult));
				}
			});

			return pPromise->get_future();
		}
	}

	PacketIoFuture ConnectToNode(const std::string& certificateDirectory, const ionet::Node& node, thread::IoThreadPool& pool) {
		return ConnectToNode(CreateToolConnectionSettings(certificateDirectory), node, pool);
	}

	PacketIoFuture ConnectToNode(const net::ConnectionSettings& connectionSettings, const ionet::Node& node, thread::IoThreadPool& pool) {
		return ConnectToNodeEx(connectionSettings, node, pool).then([](auto&& socketInfoFuture) {
			return socketInfoFuture.get().socket()->buffered();
		});
	}

	// region MultiNodeConnector

	MultiNodeConnector::MultiNodeConnector(const std::string& certificateDirectory)
			: m_certificateDirectory(certificateDirectory)
			, m_pPool(CreateStartedThreadPool())
	{}

	MultiNodeConnector::~MultiNodeConnector() {
		// wait for all thread pool work to complete in order to prevent a self-join race condition
		m_pPool->join();
	}

	thread::IoThreadPool& MultiNodeConnector::pool() {
		return *m_pPool;
	}

	PacketSocketInfoFuture MultiNodeConnector::connect(const ionet::Node& node) {
		return ConnectToNodeEx(CreateToolConnectionSettings(m_certificateDirectory), node, *m_pPool);
	}

	// endregion
}}
