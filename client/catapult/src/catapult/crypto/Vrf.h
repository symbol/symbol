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

#pragma once
#include "CryptoUtils.h"
#include "SharedKey.h"
#include "catapult/types.h"

namespace catapult { namespace crypto {

	/// VRF proof gamma.
	struct ProofGamma_tag { static constexpr size_t Size = 32; };
	using ProofGamma = utils::ByteArray<ProofGamma_tag>;

	/// VRF proof verification hash.
	struct ProofVerificationHash_tag { static constexpr size_t Size = 16; };
	using ProofVerificationHash = utils::ByteArray<ProofVerificationHash_tag>;

	/// VRF proof scalar.
	struct ProofScalar_tag { static constexpr size_t Size = 32; };
	using ProofScalar = utils::ByteArray<ProofScalar_tag>;

#pragma pack(push, 1)

	/// VRF proof for the verifiable random function.
	struct VrfProof {
		/// Gamma.
		ProofGamma Gamma;

		/// Verification hash.
		ProofVerificationHash VerificationHash;

		/// Scalar.
		ProofScalar Scalar;
	};

#pragma pack(pop)

	/// Generates a verifiable random function proof from \a alpha and \a keyPair.
	VrfProof GenerateVrfProof(const RawBuffer& alpha, const KeyPair& keyPair);

	/// Verifies verifiable random function proof (\a vrfProof) using \a alpha and \a publicKey.
	Hash512 VerifyVrfProof(const VrfProof& vrfProof, const RawBuffer& alpha, const Key& publicKey);

	/// Generates a verifiable random function proof hash from \a gamma.
	Hash512 GenerateVrfProofHash(const ProofGamma& gamma);
}}
