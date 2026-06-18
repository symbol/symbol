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

#include "ServerConnector.h"
#include "src/catapult/ionet/Node.h"
#include "src/catapult/ionet/PacketSocket.h"
#include "src/catapult/thread/IoThreadPool.h"
#include "src/catapult/thread/TimedCallback.h"
#include "src/catapult/utils/Logging.h"
#include "src/catapult/utils/SpinLock.h"
#include "src/catapult/utils/WeakContainer.h"
#include <algorithm>
#include <vector>

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

				auto pCancel = std::make_shared<action>();
				auto socketOptions = m_settings.toSocketOptions();
				const auto& endpoint = node.endpoint();
				auto cancel = ionet::Connect(m_ioContext, socketOptions, endpoint, [pThis = shared_from_this(), identityKey, pRequest, pCancel](
						auto result,
						const auto& connectedSocketInfo) {
					*pCancel = action{};
					if (ionet::ConnectResult::Connected != result)
						return pRequest->callback(PeerConnectCode::Socket_Error, ionet::PacketSocketInfo());

					pThis->verify(identityKey, connectedSocketInfo, pRequest);
				});
				*pCancel = cancel;

				{
					utils::SpinLockGuard guard(m_cancelsLock);
					m_pendingCancels.erase(
						std::remove_if(m_pendingCancels.begin(), m_pendingCancels.end(), [](const auto& p) { return !*p; }),
						m_pendingCancels.end());
					m_pendingCancels.push_back(pCancel);
				}

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

				std::vector<std::shared_ptr<action>> pendingCancels;
				{
					utils::SpinLockGuard guard(m_cancelsLock);
					pendingCancels = std::move(m_pendingCancels);
				}

				for (auto& pCancel : pendingCancels) {
					if (*pCancel)
						(*pCancel)();
				}

				m_sockets.clear();
			}

		private:
			boost::asio::io_context& m_ioContext;
			Key m_serverPublicKey;
			ConnectionSettings m_settings;

			std::string m_name;
			std::string m_tag;

			utils::SpinLock m_cancelsLock;
			std::vector<std::shared_ptr<action>> m_pendingCancels;
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
