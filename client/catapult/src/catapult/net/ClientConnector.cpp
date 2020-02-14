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

#include "ClientConnector.h"
#include "VerifyPeer.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/thread/TimedCallback.h"
#include "catapult/utils/Logging.h"
#include "catapult/utils/WeakContainer.h"

namespace catapult { namespace net {

	namespace {
		using PacketSocketPointer = std::shared_ptr<ionet::PacketSocket>;

		constexpr auto Empty_Key = Key();

		class DefaultClientConnector
				: public ClientConnector
				, public std::enable_shared_from_this<DefaultClientConnector> {
		public:
			DefaultClientConnector(
					const std::shared_ptr<thread::IoThreadPool>& pPool,
					const crypto::KeyPair& keyPair,
					const ConnectionSettings& settings,
					const std::string& name)
					: m_pPool(pPool)
					, m_keyPair(keyPair)
					, m_settings(settings)
					, m_name(name)
					, m_tag(m_name.empty() ? std::string() : " (" + m_name + ")")
					, m_sockets([](auto& socket) { socket.close(); })
			{}

		public:
			size_t numActiveConnections() const override {
				return m_sockets.size();
			}

			const std::string& name() const override {
				return m_name;
			}

		public:
			void accept(const PacketSocketPointer& pAcceptedSocket, const AcceptCallback& callback) override {
				if (!pAcceptedSocket)
					return callback(PeerConnectCode::Socket_Error, nullptr, Empty_Key);

				m_sockets.insert(pAcceptedSocket);

				auto pRequest = thread::MakeTimedCallback(
						m_pPool->ioContext(),
						callback,
						PeerConnectCode::Timed_Out,
						PacketSocketPointer(),
						Empty_Key);
				pRequest->setTimeout(m_settings.Timeout);
				pRequest->setTimeoutHandler([pAcceptedSocket]() { pAcceptedSocket->close(); });

				auto securityModes = ionet::ConnectionSecurityMode::None;
				VerifyClient(pAcceptedSocket, m_keyPair, securityModes, [pThis = shared_from_this(), pAcceptedSocket, pRequest](
						auto verifyResult,
						const auto& verifiedPeerInfo) {
					if (VerifyResult::Success != verifyResult) {
						CATAPULT_LOG(warning) << "VerifyClient failed with " << verifyResult << pThis->m_tag;
						return pRequest->callback(PeerConnectCode::Verify_Error, nullptr, Empty_Key);
					}

					if (!pThis->m_settings.AllowIncomingSelfConnections && pThis->m_keyPair.publicKey() == verifiedPeerInfo.PublicKey) {
						CATAPULT_LOG(warning) << "self accept detected and aborted" << pThis->m_tag;
						return pRequest->callback(PeerConnectCode::Self_Connection_Error, nullptr, Empty_Key);
					}

					return pRequest->callback(PeerConnectCode::Accepted, pAcceptedSocket, verifiedPeerInfo.PublicKey);
				});
			}

			void shutdown() override {
				CATAPULT_LOG(info) << "closing all connections in ClientConnector" << m_tag;
				m_sockets.clear();
			}

		private:
			std::shared_ptr<thread::IoThreadPool> m_pPool;
			const crypto::KeyPair& m_keyPair;
			ConnectionSettings m_settings;

			std::string m_name;
			std::string m_tag;

			utils::WeakContainer<ionet::PacketSocket> m_sockets;
		};
	}

	std::shared_ptr<ClientConnector> CreateClientConnector(
			const std::shared_ptr<thread::IoThreadPool>& pPool,
			const crypto::KeyPair& keyPair,
			const ConnectionSettings& settings,
			const char* name) {
		return std::make_shared<DefaultClientConnector>(pPool, keyPair, settings, name ? std::string(name) : std::string());
	}
}}
