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

#include "Challenge.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/Signer.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/utils/Logging.h"
#include "catapult/utils/RandomGenerator.h"
#include <random>

namespace catapult { namespace net {

	namespace {
		void GenerateRandomChallenge(Challenge& challenge) {
			utils::LowEntropyRandomGenerator generator;
			generator.fill(challenge.data(), challenge.size());
		}

		RawBuffer ToRawBuffer(const ionet::ConnectionSecurityMode& securityMode) {
			return { reinterpret_cast<const uint8_t*>(&securityMode), sizeof(ionet::ConnectionSecurityMode) };
		}

		void SignChallenge(const crypto::KeyPair& keyPair, std::initializer_list<const RawBuffer> buffers, Signature& computedSignature) {
			CATAPULT_LOG(debug) << "preparing challenge response";
			crypto::Sign(keyPair, buffers, computedSignature);
			CATAPULT_LOG(trace) << "signature: " << computedSignature;
		}

		bool VerifyChallenge(const Key& publicKey, const std::vector<RawBuffer>& buffers, const Signature& signature) {
			CATAPULT_LOG(trace) << "verify signature: " << signature;
			auto isVerified = crypto::Verify(publicKey, buffers, signature);
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
			const crypto::KeyPair& keyPair,
			ionet::ConnectionSecurityMode securityMode) {
		auto pResponse = ionet::CreateSharedPacket<ServerChallengeResponse>();
		GenerateRandomChallenge(pResponse->Challenge);
		SignChallenge(keyPair, { request.Challenge, ToRawBuffer(securityMode) }, pResponse->Signature);

		pResponse->PublicKey = keyPair.publicKey();
		pResponse->SecurityMode = securityMode;
		return pResponse;
	}

	bool VerifyServerChallengeResponse(const ServerChallengeResponse& response, const Challenge& challenge) {
		return VerifyChallenge(response.PublicKey, { challenge, ToRawBuffer(response.SecurityMode) }, response.Signature);
	}

	std::shared_ptr<ClientChallengeResponse> GenerateClientChallengeResponse(
			const ServerChallengeResponse& request,
			const crypto::KeyPair& keyPair) {
		auto pResponse = ionet::CreateSharedPacket<ClientChallengeResponse>();
		SignChallenge(keyPair, { request.Challenge }, pResponse->Signature);
		return pResponse;
	}

	bool VerifyClientChallengeResponse(const ClientChallengeResponse& response, const Key& serverPublicKey, const Challenge& challenge) {
		return VerifyChallenge(serverPublicKey, { challenge }, response.Signature);
	}
}}
