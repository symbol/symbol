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
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketSocket.h"
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
					thread::IoThreadPool& pool,
					const Key& serverPublicKey,
					const ConnectionSettings& settings,
					const std::string& name)
					: m_ioContext(pool.ioContext())
					, m_serverPublicKey(serverPublicKey)
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
			void connect(const ionet::Node& node, const ConnectCallback& callback) override {
				const auto& identityKey = node.identity().PublicKey;
				if (!m_settings.AllowOutgoingSelfConnections && m_serverPublicKey == identityKey) {
					CATAPULT_LOG(warning) << "self connection detected and aborted" << m_tag;
					return callback(PeerConnectCode::Self_Connection_Error, ionet::PacketSocketInfo());
				}

				auto pRequest = thread::MakeTimedCallback(m_ioContext, callback, PeerConnectCode::Timed_Out, ionet::PacketSocketInfo());
				pRequest->setTimeout(m_settings.Timeout);

				auto socketOptions = m_settings.toSocketOptions();
				const auto& endpoint = node.endpoint();
				auto cancel = ionet::Connect(m_ioContext, socketOptions, endpoint, [pThis = shared_from_this(), identityKey, pRequest](
						auto result,
						const auto& connectedSocketInfo) {
					if (ionet::ConnectResult::Connected != result)
						return pRequest->callback(PeerConnectCode::Socket_Error, ionet::PacketSocketInfo());

					pThis->verify(identityKey, connectedSocketInfo, pRequest);
				});

				pRequest->setTimeoutHandler([pThis = shared_from_this(), cancel]() {
					cancel();
					CATAPULT_LOG(debug) << "connect failed due to timeout" << pThis->m_tag;
				});
			}

		private:
			template<typename TRequest>
			void verify(
					const Key& expectedIdentityKey,
					const ionet::PacketSocketInfo& connectedSocketInfo,
					const std::shared_ptr<TRequest>& pRequest) {
				if (expectedIdentityKey != connectedSocketInfo.publicKey()) {
					CATAPULT_LOG(warning)
							<< "aborting connection with identity mismatch (expected " << expectedIdentityKey
							<< ", actual " << connectedSocketInfo.publicKey() << ")" << m_tag;
					return pRequest->callback(PeerConnectCode::Verify_Error, ionet::PacketSocketInfo());
				}

				m_sockets.insert(connectedSocketInfo.socket());
				pRequest->callback(PeerConnectCode::Accepted, connectedSocketInfo);
			}

		public:
			void shutdown() override {
				CATAPULT_LOG(info) << "closing all connections in ServerConnector" << m_tag;
				m_sockets.clear();
			}

		private:
			boost::asio::io_context& m_ioContext;
			Key m_serverPublicKey;
			ConnectionSettings m_settings;

			std::string m_name;
			std::string m_tag;

			utils::WeakContainer<ionet::PacketSocket> m_sockets;
		};
	}

	std::shared_ptr<ServerConnector> CreateServerConnector(
			thread::IoThreadPool& pool,
			const Key& serverPublicKey,
			const ConnectionSettings& settings,
			const char* name) {
		return std::make_shared<DefaultServerConnector>(pool, serverPublicKey, settings, name ? std::string(name) : std::string());
	}
}}
