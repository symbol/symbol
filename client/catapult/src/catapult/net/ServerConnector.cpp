#include "ServerConnector.h"
#include "AsyncTcpServer.h"
#include "VerifyPeer.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/thread/TimedCallback.h"
#include "catapult/utils/Logging.h"
#include "catapult/utils/WeakContainer.h"

namespace catapult { namespace net {

	namespace {
		class DefaultServerConnector
				: public ServerConnector
				, public std::enable_shared_from_this<DefaultServerConnector> {
		public:
			DefaultServerConnector(
					const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
					const crypto::KeyPair& keyPair,
					const ConnectionSettings& settings)
					: m_pPool(pPool)
					, m_keyPair(keyPair)
					, m_settings(settings)
					, m_sockets([](auto& socket) { socket.close(); })
			{}

		public:
			size_t numActiveConnections() const override {
				return m_sockets.size();
			}

		public:
			void connect(const ionet::Node& node, const ConnectCallback& callback) override {
				auto& service = m_pPool->service();
				auto pRequest = thread::MakeTimedCallback(
						service,
						callback,
						PeerConnectResult::Timed_Out,
						std::shared_ptr<ionet::PacketSocket>());
				pRequest->setTimeout(m_settings.Timeout);
				auto cancel = ionet::Connect(
						service,
						m_settings.toSocketOptions(),
						node.Endpoint,
						[pThis = shared_from_this(), node, pRequest](auto result, const auto& pSocket) {
							if (ionet::ConnectResult::Connected != result)
								return pRequest->callback(PeerConnectResult::Socket_Error, nullptr);

							pThis->verify(node.Identity.PublicKey, pSocket, pRequest);
						});

				pRequest->setTimeoutHandler([cancel]() {
					cancel();
					CATAPULT_LOG(debug) << "connect failed due to timeout";
				});
			}

		private:
			template<typename TRequest>
			void verify(
					const Key& publicKey,
					const std::shared_ptr<ionet::PacketSocket>& pSocket,
					const std::shared_ptr<TRequest>& pRequest) {
				m_sockets.insert(pSocket);
				pRequest->setTimeoutHandler([pSocket]() {
					pSocket->close();
					CATAPULT_LOG(debug) << "verify failed due to timeout";
				});

				VerifyServer(pSocket, publicKey, m_keyPair, [pThis = shared_from_this(), pSocket, pRequest](
						auto verifyResult,
						const auto&) {
					if (VerifyResult::Success != verifyResult) {
						CATAPULT_LOG(warning) << "VerifyServer failed with " << verifyResult;
						return pRequest->callback(PeerConnectResult::Verify_Error, nullptr);
					}

					return pRequest->callback(PeerConnectResult::Accepted, pSocket);
				});
			}

		public:
			void shutdown() override {
				CATAPULT_LOG(info) << "closing all connections in ServerConnector";
				m_sockets.clear();
			}

		private:
			std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
			const crypto::KeyPair& m_keyPair;
			ConnectionSettings m_settings;
			utils::WeakContainer<ionet::PacketSocket> m_sockets;
		};
	}

	std::shared_ptr<ServerConnector> CreateServerConnector(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings) {
		return std::make_shared<DefaultServerConnector>(pPool, keyPair, settings);
	}
}}
