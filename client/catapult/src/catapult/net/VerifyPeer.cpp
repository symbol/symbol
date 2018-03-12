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

	namespace {
		class VerifyClientHandler : public std::enable_shared_from_this<VerifyClientHandler> {
		public:
			VerifyClientHandler(
					const std::shared_ptr<ionet::PacketIo>& pIo,
					const crypto::KeyPair& keyPair,
					const VerifyCallback& callback)
					: m_pIo(pIo)
					, m_keyPair(keyPair)
					, m_callback(callback)
			{}

		public:
			void start() {
				m_pRequest = GenerateServerChallengeRequest();
				m_pIo->write(m_pRequest, [pThis = shared_from_this()](auto code) {
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
				if (nullptr == pResponse)
					return invokeCallback(VerifyResult::Malformed_Data);

				if (!VerifyServerChallengeResponse(*pResponse, m_pRequest->Challenge))
					return invokeCallback(VerifyResult::Failure_Challenge);

				const auto& clientPublicKey = pResponse->PublicKey;
				auto pServerResponse = GenerateClientChallengeResponse(*pResponse, m_keyPair);
				m_pIo->write(pServerResponse, [pThis = shared_from_this(), clientPublicKey](auto writeCode) {
					pThis->handleClientChallengeReponseWrite(writeCode, clientPublicKey);
				});
			}

			void handleClientChallengeReponseWrite(ionet::SocketOperationCode code, const Key& clientPublicKey) const {
				if (ionet::SocketOperationCode::Success != code)
					return invokeCallback(VerifyResult::Io_Error_ClientChallengeResponse, clientPublicKey);

				invokeCallback(VerifyResult::Success, clientPublicKey);
			}

		private:
			void invokeCallback(VerifyResult result) const {
				invokeCallback(result, Key());
			}

			void invokeCallback(VerifyResult result, const Key& clientPublicKey) const {
				CATAPULT_LOG(debug) << "VerifyClient completed with " << result;
				m_callback(result, clientPublicKey);
			}

		private:
			std::shared_ptr<ionet::PacketIo> m_pIo;
			const crypto::KeyPair& m_keyPair;
			VerifyCallback m_callback;
			std::shared_ptr<ServerChallengeRequest> m_pRequest;
		};
	}

	void VerifyClient(const std::shared_ptr<ionet::PacketIo>& pClientIo, const crypto::KeyPair& keyPair, const VerifyCallback& callback) {
		auto pHandler = std::make_shared<VerifyClientHandler>(pClientIo, keyPair, callback);
		pHandler->start();
	}

	namespace {
		class VerifyServerHandler : public std::enable_shared_from_this<VerifyServerHandler> {
		public:
			VerifyServerHandler(
					const std::shared_ptr<ionet::PacketIo>& pIo,
					const Key& serverPublicKey,
					const crypto::KeyPair& keyPair,
					const VerifyCallback& callback)
					: m_pIo(pIo)
					, m_serverPublicKey(serverPublicKey)
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
				if (nullptr == pRequest)
					return invokeCallback(VerifyResult::Malformed_Data);

				m_pRequest = GenerateServerChallengeResponse(*pRequest, m_keyPair);
				m_pIo->write(m_pRequest, [pThis = shared_from_this()](auto writeCode) {
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
				if (nullptr == pResponse)
					return invokeCallback(VerifyResult::Malformed_Data);

				auto isVerified = VerifyClientChallengeResponse(*pResponse, m_serverPublicKey, m_pRequest->Challenge);
				invokeCallback(isVerified ? VerifyResult::Success: VerifyResult::Failure_Challenge);
			}

		private:
			void invokeCallback(VerifyResult result) const {
				CATAPULT_LOG(debug) << "VerifyServer completed with " << result;
				m_callback(result, m_serverPublicKey);
			}

		private:
			std::shared_ptr<ionet::PacketIo> m_pIo;
			Key m_serverPublicKey;
			const crypto::KeyPair& m_keyPair;
			VerifyCallback m_callback;
			std::shared_ptr<ClientChallengeRequest> m_pRequest;
		};
	}

	void VerifyServer(
			const std::shared_ptr<ionet::PacketIo>& pServerIo,
			const Key& serverPublicKey,
			const crypto::KeyPair& keyPair,
			const VerifyCallback& callback) {
		auto pHandler = std::make_shared<VerifyServerHandler>(pServerIo, serverPublicKey, keyPair, callback);
		pHandler->start();
	}
}}
