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
#include "catapult/crypto/KeyUtils.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/ServerConnector.h"
#include "catapult/thread/IoServiceThreadPool.h"

namespace catapult { namespace tools {

	namespace {
		ionet::Node CreateLocalNode(const Key& serverPublicKey) {
			return ionet::Node(serverPublicKey, { "127.0.0.1", 7900 }, { model::NetworkIdentifier::Zero, "tool endpoint" });
		}
	}

	PacketIoFuture ConnectToLocalNode(
			const crypto::KeyPair& clientKeyPair,
			const Key& serverPublicKey,
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool) {
		return ConnectToNode(clientKeyPair, CreateLocalNode(serverPublicKey), pPool);
	}

	namespace {
		std::shared_ptr<ionet::PacketIo> CreateBufferedPacketIo(
				const std::shared_ptr<ionet::PacketSocket>& pSocket,
				const std::shared_ptr<thread::IoServiceThreadPool>& pPool) {
			// attach the lifetime of the pool to the returned io in order to prevent it from being destroyed before the io
			auto pBufferedIo = pSocket->buffered();
			return std::shared_ptr<ionet::PacketIo>(pBufferedIo.get(), [pBufferedIo, pPool](const auto*) {});
		}

		auto MakePeerConnectException(const ionet::Node& node, net::PeerConnectResult connectResult) {
			std::ostringstream out;
			out << "connecting to " << node << " failed with " << connectResult;
			return std::make_exception_ptr(catapult_runtime_error(out.str().c_str()));
		}
	}

	PacketIoFuture ConnectToNode(
			const crypto::KeyPair& clientKeyPair,
			const ionet::Node& node,
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool) {
		auto pPromise = std::make_shared<thread::promise<std::shared_ptr<ionet::PacketIo>>>();

		auto pConnector = net::CreateServerConnector(pPool, clientKeyPair, net::ConnectionSettings());
		pConnector->connect(node, [node, pPool, pPromise](auto connectResult, const auto& pPacketSocket) {
			switch (connectResult) {
			case net::PeerConnectResult::Accepted:
				return pPromise->set_value(CreateBufferedPacketIo(pPacketSocket, pPool));

			case net::PeerConnectResult::Verify_Error:
				CATAPULT_LOG(fatal)
						<< "verification problem - client expected following public key from the server: "
						<< crypto::FormatKey(node.identityKey());
				return pPromise->set_exception(MakePeerConnectException(node, connectResult));

			default:
				CATAPULT_LOG(fatal) << "error occurred when trying to connect to node: " << connectResult;
				return pPromise->set_exception(MakePeerConnectException(node, connectResult));
			}
		});

		return pPromise->get_future();
	}

	// region MultiNodeConnector

	MultiNodeConnector::MultiNodeConnector()
			: m_clientKeyPair(GenerateRandomKeyPair())
			, m_pPool(CreateStartedThreadPool())
	{}

	MultiNodeConnector::~MultiNodeConnector() {
		// wait for all thread pool work to complete in order to prevent a self-join race condition
		m_pPool->join();
	}

	thread::IoServiceThreadPool& MultiNodeConnector::pool() {
		return *m_pPool;
	}

	PacketIoFuture MultiNodeConnector::connect(const ionet::Node& node) {
		return ConnectToNode(m_clientKeyPair, node, m_pPool);
	}

	// endregion
}}
