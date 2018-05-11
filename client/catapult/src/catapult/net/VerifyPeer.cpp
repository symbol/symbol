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

#include "VerifyPeer.h"
#include "Challenge.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"
#include <random>

namespace catapult { namespace net {

#define DEFINE_ENUM VerifyResult
#define ENUM_LIST VERIFY_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

	// VerifyClient: the server (verifies client connections)
	// VerifyServer: the client (verifies server connections)
	// SERVER -> ServerChallengeRequest  -> CLIENT
	// SERVER <- ServerChallengeResponse <- CLIENT
	// SERVER -> ClientChallengeResponse -> CLIENT

	namespace {
		class VerifyClientHandler : public std::enable_shared_from_this<VerifyClientHandler> {
		public:
			VerifyClientHandler(
					const std::shared_ptr<ionet::PacketIo>& pIo,
					const crypto::KeyPair& keyPair,
					ionet::ConnectionSecurityMode allowedSecurityModes,
					const VerifyCallback& callback)
					: m_pIo(pIo)
					, m_keyPair(keyPair)
					, m_allowedSecurityModes(allowedSecurityModes)
					, m_callback(callback)
			{}

		public:
			void start() {
				m_pRequest = GenerateServerChallengeRequest();
				m_pIo->write(ionet::PacketPayload(m_pRequest), [pThis = shared_from_this()](auto code) {
					pThis->handleServerChallengeRequestWrite(code);
				});
			}

		private:
			void handleServerChallengeRequestWrite(ionet::SocketOperationCode code) const {
				if (ionet::SocketOperationCode::Success != code)
					return invokeCallback(VerifyResult::Io_Error_ServerChallengeRequest);

				m_pIo->read([pThis = shared_from_this()](auto readCode, const auto* pPacket) {
					pThis->handleServerChallengeResponseRead(readCode, pPacket);
				});
			}

			void handleServerChallengeResponseRead(ionet::SocketOperationCode code, const ionet::Packet* pPacket) const {
				if (ionet::SocketOperationCode::Success != code)
					return invokeCallback(VerifyResult::Io_Error_ServerChallengeResponse);

				const auto* pResponse = ionet::CoercePacket<ServerChallengeResponse>(pPacket);
				if (!pResponse)
					return invokeCallback(VerifyResult::Malformed_Data);

				auto clientPeerInfo = VerifiedPeerInfo{ pResponse->PublicKey, pResponse->SecurityMode };
				if (!HasSingleFlag(pResponse->SecurityMode) || !HasFlag(pResponse->SecurityMode, m_allowedSecurityModes))
					return invokeCallback(VerifyResult::Failure_Unsupported_Connection, clientPeerInfo);

				if (!VerifyServerChallengeResponse(*pResponse, m_pRequest->Challenge))
					return invokeCallback(VerifyResult::Failure_Challenge, clientPeerInfo);

				auto pServerResponse = GenerateClientChallengeResponse(*pResponse, m_keyPair);
				m_pIo->write(ionet::PacketPayload(pServerResponse), [pThis = shared_from_this(), clientPeerInfo](auto writeCode) {
					pThis->handleClientChallengeReponseWrite(writeCode, clientPeerInfo);
				});
			}

			void handleClientChallengeReponseWrite(ionet::SocketOperationCode code, const VerifiedPeerInfo& clientPeerInfo) const {
				if (ionet::SocketOperationCode::Success != code)
					return invokeCallback(VerifyResult::Io_Error_ClientChallengeResponse, clientPeerInfo);

				invokeCallback(VerifyResult::Success, clientPeerInfo);
			}

		private:
			void invokeCallback(VerifyResult result) const {
				invokeCallback(result, VerifiedPeerInfo());
			}

			void invokeCallback(VerifyResult result, const VerifiedPeerInfo& clientPeerInfo) const {
				CATAPULT_LOG(debug) << "VerifyClient completed with " << result << " (" << clientPeerInfo.SecurityMode << ")";
				m_callback(result, clientPeerInfo);
			}

		private:
			std::shared_ptr<ionet::PacketIo> m_pIo;
			const crypto::KeyPair& m_keyPair;
			ionet::ConnectionSecurityMode m_allowedSecurityModes;
			VerifyCallback m_callback;
			std::shared_ptr<ServerChallengeRequest> m_pRequest;
		};
	}

	void VerifyClient(
			const std::shared_ptr<ionet::PacketIo>& pClientIo,
			const crypto::KeyPair& keyPair,
			ionet::ConnectionSecurityMode allowedSecurityModes,
			const VerifyCallback& callback) {
		auto pHandler = std::make_shared<VerifyClientHandler>(pClientIo, keyPair, allowedSecurityModes, callback);
		pHandler->start();
	}

	namespace {
		class VerifyServerHandler : public std::enable_shared_from_this<VerifyServerHandler> {
		public:
			VerifyServerHandler(
					const std::shared_ptr<ionet::PacketIo>& pIo,
					const VerifiedPeerInfo& serverPeerInfo,
					const crypto::KeyPair& keyPair,
					const VerifyCallback& callback)
					: m_pIo(pIo)
					, m_serverPeerInfo(serverPeerInfo)
					, m_keyPair(keyPair)
					, m_callback(callback)
			{}

		public:
			void start() {
				m_pIo->read([pThis = shared_from_this()](auto code, const auto* pPacket) {
					pThis->handleServerChallengeRequestRead(code, pPacket);
				});
			}

		private:
			void handleServerChallengeRequestRead(ionet::SocketOperationCode code, const ionet::Packet* pPacket) {
				if (ionet::SocketOperationCode::Success != code)
					return invokeCallback(VerifyResult::Io_Error_ServerChallengeRequest);

				const auto* pRequest = ionet::CoercePacket<ServerChallengeRequest>(pPacket);
				if (!pRequest)
					return invokeCallback(VerifyResult::Malformed_Data);

				m_pRequest = GenerateServerChallengeResponse(*pRequest, m_keyPair, m_serverPeerInfo.SecurityMode);
				m_pIo->write(ionet::PacketPayload(m_pRequest), [pThis = shared_from_this()](auto writeCode) {
					pThis->handleServerChallengeResponseWrite(writeCode);
				});
			}

			void handleServerChallengeResponseWrite(ionet::SocketOperationCode code) const {
				if (ionet::SocketOperationCode::Success != code)
					return invokeCallback(VerifyResult::Io_Error_ServerChallengeResponse);

				m_pIo->read([pThis = shared_from_this()](auto readCode, const auto* pPacket) {
					pThis->handleClientChallengeReponseRead(readCode, pPacket);
				});
			}

			void handleClientChallengeReponseRead(ionet::SocketOperationCode code, const ionet::Packet* pPacket) const {
				if (ionet::SocketOperationCode::Success != code)
					return invokeCallback(VerifyResult::Io_Error_ClientChallengeResponse);

				const auto* pResponse = ionet::CoercePacket<ClientChallengeResponse>(pPacket);
				if (!pResponse)
					return invokeCallback(VerifyResult::Malformed_Data);

				auto isVerified = VerifyClientChallengeResponse(*pResponse, m_serverPeerInfo.PublicKey, m_pRequest->Challenge);
				invokeCallback(isVerified ? VerifyResult::Success: VerifyResult::Failure_Challenge);
			}

		private:
			void invokeCallback(VerifyResult result) const {
				CATAPULT_LOG(debug) << "VerifyServer completed with " << result;
				m_callback(result, m_serverPeerInfo);
			}

		private:
			std::shared_ptr<ionet::PacketIo> m_pIo;
			VerifiedPeerInfo m_serverPeerInfo;
			const crypto::KeyPair& m_keyPair;
			VerifyCallback m_callback;
			std::shared_ptr<ServerChallengeResponse> m_pRequest;
		};
	}

	void VerifyServer(
			const std::shared_ptr<ionet::PacketIo>& pServerIo,
			const VerifiedPeerInfo& serverPeerInfo,
			const crypto::KeyPair& keyPair,
			const VerifyCallback& callback) {
		auto pHandler = std::make_shared<VerifyServerHandler>(pServerIo, serverPeerInfo, keyPair, callback);
		pHandler->start();
	}
}}
