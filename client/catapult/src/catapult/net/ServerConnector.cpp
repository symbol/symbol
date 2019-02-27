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

#include "ServerConnector.h"
#include "VerifyPeer.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/ionet/SecurePacketSocketDecorator.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/thread/TimedCallback.h"
#include "catapult/utils/Logging.h"
#include "catapult/utils/WeakContainer.h"

namespace catapult { namespace net {

	namespace {
		using PacketSocketPointer = std::shared_ptr<ionet::PacketSocket>;

		class DefaultServerConnector
				: public ServerConnector
				, public std::enable_shared_from_this<DefaultServerConnector> {
		public:
			DefaultServerConnector(
					const std::shared_ptr<thread::IoThreadPool>& pPool,
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
				auto& ioContext = m_pPool->ioContext();
				auto pRequest = thread::MakeTimedCallback(ioContext, callback, PeerConnectCode::Timed_Out, PacketSocketPointer());
				pRequest->setTimeout(m_settings.Timeout);
				auto cancel = ionet::Connect(
						ioContext,
						m_settings.toSocketOptions(),
						node.endpoint(),
						[pThis = shared_from_this(), node, pRequest](auto result, const auto& pConnectedSocket) {
							if (ionet::ConnectResult::Connected != result)
								return pRequest->callback(PeerConnectCode::Socket_Error, nullptr);

							pThis->verify(node.identityKey(), pConnectedSocket, pRequest);
						});

				pRequest->setTimeoutHandler([cancel]() {
					cancel();
					CATAPULT_LOG(debug) << "connect failed due to timeout";
				});
			}

		private:
			template<typename TRequest>
			void verify(const Key& publicKey, const PacketSocketPointer& pConnectedSocket, const std::shared_ptr<TRequest>& pRequest) {
				m_sockets.insert(pConnectedSocket);
				pRequest->setTimeoutHandler([pConnectedSocket]() {
					pConnectedSocket->close();
					CATAPULT_LOG(debug) << "verify failed due to timeout";
				});

				VerifiedPeerInfo serverPeerInfo{ publicKey, m_settings.OutgoingSecurityMode };
				VerifyServer(pConnectedSocket, serverPeerInfo, m_keyPair, [pThis = shared_from_this(), pConnectedSocket, pRequest](
						auto verifyResult,
						const auto& verifiedPeerInfo) {
					if (VerifyResult::Success != verifyResult) {
						CATAPULT_LOG(warning) << "VerifyServer failed with " << verifyResult;
						return pRequest->callback(PeerConnectCode::Verify_Error, nullptr);
					}

					auto pSecuredSocket = pThis->secure(pConnectedSocket, verifiedPeerInfo);
					return pRequest->callback(PeerConnectCode::Accepted, pSecuredSocket);
				});
			}

			PacketSocketPointer secure(const PacketSocketPointer& pSocket, const VerifiedPeerInfo& peerInfo) {
				return Secure(pSocket, peerInfo.SecurityMode, m_keyPair, peerInfo.PublicKey, m_settings.MaxPacketDataSize);
			}

		public:
			void shutdown() override {
				CATAPULT_LOG(info) << "closing all connections in ServerConnector";
				m_sockets.clear();
			}

		private:
			std::shared_ptr<thread::IoThreadPool> m_pPool;
			const crypto::KeyPair& m_keyPair;
			ConnectionSettings m_settings;
			utils::WeakContainer<ionet::PacketSocket> m_sockets;
		};
	}

	std::shared_ptr<ServerConnector> CreateServerConnector(
			const std::shared_ptr<thread::IoThreadPool>& pPool,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings) {
		return std::make_shared<DefaultServerConnector>(pPool, keyPair, settings);
	}
}}
