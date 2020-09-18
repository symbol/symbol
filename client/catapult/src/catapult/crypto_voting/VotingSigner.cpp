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

#include "VotingSigner.h"
#include "catapult/crypto/Signer.h"

namespace catapult { namespace crypto {

#define SIGNATURE_16_BYTE_PADDING "\xCA\xCA\xCA\xCA\xCA\xCA\xCA\xCA\xCA\xCA\xCA\xCA\xCA\xCA\xCA\xCA"

	namespace {
		constexpr const char* Signature_Padding = SIGNATURE_16_BYTE_PADDING SIGNATURE_16_BYTE_PADDING;
	}

	void Sign(const VotingKeyPair& keyPair, const RawBuffer& dataBuffer, VotingSignature& computedSignature) {
		Sign(keyPair, { dataBuffer }, computedSignature);
	}

	void Sign(const VotingKeyPair& keyPair, std::initializer_list<const RawBuffer> buffersList, VotingSignature& computedSignature) {
		Signature ed25519Signature;
		auto ed25519KeyPair = KeyPair::FromPrivate(PrivateKey::FromBuffer(keyPair.privateKey()));
		Sign(ed25519KeyPair, buffersList, ed25519Signature);

		computedSignature = ed25519Signature.copyTo<VotingSignature>();
		std::memcpy(computedSignature.data() + Signature::Size, Signature_Padding, VotingSignature::Size - Signature::Size);
	}

	bool Verify(const VotingKey& publicKey, const RawBuffer& dataBuffer, const VotingSignature& signature) {
		return Verify(publicKey, std::vector<RawBuffer>{ dataBuffer }, signature);
	}

	bool Verify(const VotingKey& publicKey, const std::vector<RawBuffer>& buffersList, const VotingSignature& signature) {
		auto ed25519PublicKey = publicKey.copyTo<Key>();
		auto ed25519Signature = signature.copyTo<Signature>();

		if (!Verify(ed25519PublicKey, buffersList, ed25519Signature))
			return false;

		return 0 == std::memcmp(signature.data() + Signature::Size, Signature_Padding, VotingSignature::Size - Signature::Size);
	}
}}
