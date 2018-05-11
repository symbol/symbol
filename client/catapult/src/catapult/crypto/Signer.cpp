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

#include "Signer.h"
#include "CryptoUtils.h"
#include "Hashes.h"
#include "catapult/exceptions.h"
#include <cstring>
#include <ref10/crypto_verify_32.h>

extern "C" {
#include <ref10/ge.h>
#include <ref10/sc.h>
}

#ifdef _MSC_VER
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

namespace catapult { namespace crypto {

	namespace {
		const size_t Encoded_Size = Signature_Size / 2;
		static_assert(Encoded_Size * 2 == Hash512_Size, "hash must be big enough to hold two encoded elements");

		/// Indicates that the encoded S part of the signature is less than the group order.
		constexpr int Is_Reduced = 1;

		/// Indicates that the encoded S part of the signature is zero.
		constexpr int Is_Zero = 2;

		int ValidateEncodedSPart(const uint8_t* encodedS) {
			uint8_t encodedBuf[Signature_Size];
			uint8_t *RESTRICT encodedTempR = encodedBuf;
			uint8_t *RESTRICT encodedZero = encodedBuf + Encoded_Size;

			std::memset(encodedZero, 0, Encoded_Size);
			if (0 == std::memcmp(encodedS, encodedZero, Encoded_Size))
				return Is_Zero | Is_Reduced;

			std::memcpy(encodedTempR, encodedS, Encoded_Size);
			sc_reduce(encodedBuf);

			return std::memcmp(encodedTempR, encodedS, Encoded_Size) ? 0 : Is_Reduced;
		}

		bool IsCanonicalS(const uint8_t* encodedS) {
			return Is_Reduced == ValidateEncodedSPart(encodedS);
		}

		void CheckEncodedS(const uint8_t* encodedS) {
			if (0 == (ValidateEncodedSPart(encodedS) & Is_Reduced))
				CATAPULT_THROW_OUT_OF_RANGE("S part of signature invalid");
		}
	}

	void Sign(const KeyPair& keyPair, const RawBuffer& dataBuffer, Signature& computedSignature) {
		Sign(keyPair, { dataBuffer }, computedSignature);
	}

	void Sign(const KeyPair& keyPair, std::initializer_list<const RawBuffer> buffersList, Signature& computedSignature) {
		uint8_t *RESTRICT encodedR = computedSignature.data();
		uint8_t *RESTRICT encodedS = computedSignature.data() + Encoded_Size;

		// Hash the private key to improve randomness.
		Hash512 privHash;
		HashPrivateKey(keyPair.privateKey(), privHash);

		// r = H(privHash[256:512] || data)
		// "EdDSA avoids these issues by generating r = H(h_b, ..., h_2b?1, M), so that
		//  different messages will lead to different, hard-to-predict values of r."
		Hash512 r;
		Sha3_512_Builder sha3_r;
		sha3_r.update({ privHash.data() + Hash512_Size / 2, Hash512_Size / 2 });
		sha3_r.update(buffersList);
		sha3_r.final(r);

		// Reduce size of r since we are calculating mod group order anyway
		sc_reduce(r.data());

		// R = rModQ * base point
		ge_p3 rMulBase;
		ge_scalarmult_base(&rMulBase, r.data());
		ge_p3_tobytes(encodedR, &rMulBase);

		// h = H(encodedR || public || data)
		Hash512 h;
		Sha3_512_Builder sha3_h;
		sha3_h.update({ { encodedR, Encoded_Size }, keyPair.publicKey() });
		sha3_h.update(buffersList);
		sha3_h.final(h);

		// h = h mod group order
		sc_reduce(h.data());

		// a = fieldElement(privHash[0:256])
		privHash[0] &= 0xF8;
		privHash[31] &= 0x7F;
		privHash[31] |= 0x40;

		// S = (r + h * a) mod group order
		sc_muladd(encodedS, h.data(), privHash.data(), r.data());

		// Signature is (encodedR, encodedS)

		// Throw if encodedS is not less than the group order, don't fail in case encodedS == 0
		// (this should only throw if there is a bug in the signing code)
		CheckEncodedS(encodedS);
	}

	bool Verify(const Key& publicKey, const RawBuffer& dataBuffer, const Signature& signature) {
		return Verify(publicKey, { dataBuffer }, signature);
	}

	bool Verify(const Key& publicKey, std::initializer_list<const RawBuffer> buffersList, const Signature& signature) {
		const uint8_t *RESTRICT encodedR = signature.data();
		const uint8_t *RESTRICT encodedS = signature.data() + Encoded_Size;

		// reject if not canonical
		if (!IsCanonicalS(encodedS))
			return false;

		// reject zero public key, which is known weak key
		const Key Zero_Key{};
		if (Zero_Key == publicKey)
			return false;

		// h = H(encodedR || public || data)
		Hash512 h;
		Sha3_512_Builder sha3_h;
		sha3_h.update({ { encodedR, Encoded_Size }, publicKey });
		sha3_h.update(buffersList);
		sha3_h.final(h);

		// h = h mod group order
		sc_reduce(h.data());

		// A = -pub
		ge_p3 A;
		if (0 != ge_frombytes_negate_vartime(&A, publicKey.data()))
			return false;

		// R = encodedS * B - h * A
		ge_p2 R;
		ge_double_scalarmult_vartime(&R, h.data(), &A, encodedS);

		// Compare calculated R to given R.
		unsigned char checkr[Encoded_Size];
		ge_tobytes(checkr, &R);
		return 0 == crypto_verify_32(checkr, encodedR);
	}
}}
