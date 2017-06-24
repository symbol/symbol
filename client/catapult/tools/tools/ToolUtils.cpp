#include "ToolUtils.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/net/ServerConnector.h"
#include "catapult/plugins/TransferTransactionPlugins.h"
#include "catapult/thread/Future.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include <iostream>
#include <thread>

namespace catapult { namespace tools {

	namespace {
		ionet::Node CreateLocalNode(const Key& serverPublicKey) {
			return ionet::Node(
					{ "127.0.0.1", 7900 },
					{ serverPublicKey, "tool endpoint" },
					model::NetworkIdentifier::Zero);
		}
	}

	std::shared_ptr<thread::IoServiceThreadPool> CreateStartedThreadPool(uint32_t numThreads) {
		auto pPool = thread::CreateIoServiceThreadPool(numThreads);
		pPool->start();
		return std::move(pPool);
	}

	PacketIoFuture ConnectToLocalNode(
			const crypto::KeyPair& clientKeyPair,
			const Key& serverPublicKey,
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool) {
		return ConnectToNode(clientKeyPair, CreateLocalNode(serverPublicKey), pPool);
	}

	namespace {
		std::shared_ptr<ionet::PacketIo> CreateBufferedPacketIo(
				const std::shared_ptr<ionet::PacketIo>& pIo,
				const std::shared_ptr<thread::IoServiceThreadPool>& pPool) {
			// attach the lifetime of the pool to the returned io in order to prevent it from being destroyed before the io
			auto pBufferedIo = ionet::CreateBufferedPacketIo(pIo, boost::asio::strand(pPool->service()));
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
				CATAPULT_LOG(fatal) << "verification problem - client expected following public key from the server: "
						<< crypto::FormatKey(node.Identity.PublicKey);
				return pPromise->set_exception(MakePeerConnectException(node, connectResult));

			default:
				CATAPULT_LOG(fatal) << "error occurred when trying to connect to node: " << connectResult;
				return pPromise->set_exception(MakePeerConnectException(node, connectResult));
			}
		});

		return pPromise->get_future();
	}

	std::unique_ptr<model::TransactionRegistry> CreateDefaultTransactionRegistry() {
		auto pRegistry = std::make_unique<model::TransactionRegistry>();
		pRegistry->registerPlugin(plugins::CreateTransferTransactionPlugin());
		return pRegistry;
	}
}}
