#include "ClientConnector.h"
#include "VerifyPeer.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/thread/TimedCallback.h"
#include "catapult/utils/Logging.h"
#include "catapult/utils/WeakContainer.h"

namespace catapult { namespace net {

	namespace {
		constexpr auto Empty_Key = Key();

		class DefaultClientConnector
				: public ClientConnector
				, public std::enable_shared_from_this<DefaultClientConnector> {
		public:
			explicit DefaultClientConnector(
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

			void accept(const std::shared_ptr<ionet::PacketSocket>& pPacketSocket, const AcceptCallback& callback) override {
				if (!pPacketSocket)
					return callback(PeerConnectResult::Socket_Error, Empty_Key);

				m_sockets.insert(pPacketSocket);

				auto pRequest = thread::MakeTimedCallback(m_pPool->service(), callback, PeerConnectResult::Timed_Out, Empty_Key);
				pRequest->setTimeout(m_settings.Timeout);
				pRequest->setTimeoutHandler([pPacketSocket]() { pPacketSocket->close(); });
				VerifyClient(pPacketSocket, m_keyPair, [pThis = shared_from_this(), pPacketSocket, pRequest](
						auto verifyResult,
						const auto& key) {
					if (VerifyResult::Success != verifyResult) {
						CATAPULT_LOG(warning) << "VerifyClient failed with " << verifyResult;
						return pRequest->callback(PeerConnectResult::Verify_Error, Empty_Key);
					}

					return pRequest->callback(PeerConnectResult::Accepted, key);
				});
			}

			void shutdown() override {
				CATAPULT_LOG(info) << "closing all connections in ClientConnector";
				m_sockets.clear();
			}

		private:
			std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
			const crypto::KeyPair& m_keyPair;
			ConnectionSettings m_settings;
			utils::WeakContainer<ionet::PacketSocket> m_sockets;
		};
	}

	std::shared_ptr<ClientConnector> CreateClientConnector(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings) {
		return std::make_shared<DefaultClientConnector>(pPool, keyPair, settings);
	}
}}
