#include "Challenge.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/Signer.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/utils/Logging.h"
#include <random>

namespace catapult { namespace net {

	namespace {
		template<typename T>
		auto HexFormat(const T& container) {
			return utils::HexFormat(container, ' ');
		}

		void GenerateRandomChallenge(Challenge& challenge) {
			std::random_device generator;
			std::generate_n(challenge.begin(), challenge.size(), std::ref(generator));
		}

		void Sign(const crypto::KeyPair& keyPair, const Challenge& challenge, Signature& computedSignature) {
			CATAPULT_LOG(debug) << "preparing challenge response";
			crypto::Sign(keyPair, challenge, computedSignature);
			CATAPULT_LOG(trace) << "data: " << HexFormat(challenge);
			CATAPULT_LOG(trace) << "signature: " << HexFormat(computedSignature);
		}

		bool Verify(const Key& publicKey, const Challenge& challenge, const Signature& signature) {
			CATAPULT_LOG(trace) << "verify signature: " << HexFormat(signature);
			auto isVerified = crypto::Verify(publicKey, challenge, signature);
			CATAPULT_LOG(debug) << "verify signature result: " << isVerified;
			return isVerified;
		}
	}

	std::shared_ptr<ServerChallengeRequest> GenerateServerChallengeRequest() {
		auto pRequest = ionet::CreateSharedPacket<ServerChallengeRequest>();
		GenerateRandomChallenge(pRequest->Challenge);
		return pRequest;
	}

	std::shared_ptr<ServerChallengeResponse> GenerateServerChallengeResponse(
			const ServerChallengeRequest& request,
			const crypto::KeyPair& keyPair) {
		auto pResponse = ionet::CreateSharedPacket<ServerChallengeResponse>();
		GenerateRandomChallenge(pResponse->Challenge);
		Sign(keyPair, request.Challenge, pResponse->Signature);
		pResponse->PublicKey = keyPair.publicKey();
		return pResponse;
	}

	bool VerifyServerChallengeResponse(
			const ServerChallengeResponse& response,
			const Challenge& challenge) {
		return Verify(response.PublicKey, challenge, response.Signature);
	}

	std::shared_ptr<ClientChallengeResponse> GenerateClientChallengeResponse(
			const ClientChallengeRequest& request,
			const crypto::KeyPair& keyPair) {
		auto pResponse = ionet::CreateSharedPacket<ClientChallengeResponse>();
		Sign(keyPair, request.Challenge, pResponse->Signature);
		return pResponse;
	}

	bool VerifyClientChallengeResponse(
			const ClientChallengeResponse& response,
			const Key& serverPublicKey,
			const Challenge& challenge) {
		return Verify(serverPublicKey, challenge, response.Signature);
	}
}}
