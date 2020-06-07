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
					thread::IoThreadPool&,
					const Key& serverPublicKey,
					const ConnectionSettings& settings,
					const std::string& name)
					: m_serverPublicKey(serverPublicKey)
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
			void accept(const ionet::PacketSocketInfo& acceptedSocketInfo, const AcceptCallback& callback) override {
				if (!acceptedSocketInfo)
					return callback(PeerConnectCode::Socket_Error, nullptr, Empty_Key);

				if (!m_settings.AllowIncomingSelfConnections && m_serverPublicKey == acceptedSocketInfo.publicKey()) {
					CATAPULT_LOG(warning) << "self accept detected and aborted" << m_tag;
					return callback(PeerConnectCode::Self_Connection_Error, nullptr, Empty_Key);
				}

				auto pAcceptedSocket = acceptedSocketInfo.socket();
				m_sockets.insert(pAcceptedSocket);
				callback(PeerConnectCode::Accepted, pAcceptedSocket, acceptedSocketInfo.publicKey());
			}

			void shutdown() override {
				CATAPULT_LOG(info) << "closing all connections in ClientConnector" << m_tag;
				m_sockets.clear();
			}

		private:
			Key m_serverPublicKey;
			ConnectionSettings m_settings;

			std::string m_name;
			std::string m_tag;

			utils::WeakContainer<ionet::PacketSocket> m_sockets;
		};
	}

	std::shared_ptr<ClientConnector> CreateClientConnector(
			thread::IoThreadPool& pool,
			const Key& serverPublicKey,
			const ConnectionSettings& settings,
			const char* name) {
		return std::make_shared<DefaultClientConnector>(pool, serverPublicKey, settings, name ? std::string(name) : std::string());
	}
}}
