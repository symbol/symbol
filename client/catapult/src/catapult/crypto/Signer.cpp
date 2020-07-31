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
#include "SecureZero.h"
#include "catapult/exceptions.h"
#include <donna/catapult.h>

#ifdef _MSC_VER
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

namespace catapult { namespace crypto {

	namespace {
		const size_t Encoded_Size = Signature::Size / 2;
		static_assert(Encoded_Size * 2 == Hash512::Size, "hash must be big enough to hold two encoded elements");

		// indicates that the encoded S part of the signature is less than the group order
		constexpr int Is_Reduced = 1;

		// indicates that the encoded S part of the signature is zero
		constexpr int Is_Zero = 2;

		void Reduce(uint8_t* out, const uint8_t* encodedS) {
			bignum256modm temp;
			expand_raw256_modm(temp, encodedS);
			reduce256_modm(temp);
			contract256_modm(out, temp);
		}

		int ValidateEncodedSPart(const uint8_t* encodedS) {
			uint8_t encodedBuf[Signature::Size];
			uint8_t *RESTRICT encodedTempR = encodedBuf;
			uint8_t *RESTRICT encodedZero = encodedBuf + Encoded_Size;

			std::memset(encodedZero, 0, Encoded_Size);
			if (0 == std::memcmp(encodedS, encodedZero, Encoded_Size))
				return Is_Zero | Is_Reduced;

			Reduce(encodedTempR, encodedS);

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

	// region Sign

	void Sign(const KeyPair& keyPair, const RawBuffer& dataBuffer, Signature& computedSignature) {
		Sign(keyPair, { dataBuffer }, computedSignature);
	}

	void Sign(const KeyPair& keyPair, std::initializer_list<const RawBuffer> buffersList, Signature& computedSignature) {
		uint8_t *RESTRICT encodedR = computedSignature.data();
		uint8_t *RESTRICT encodedS = computedSignature.data() + Encoded_Size;

		// r = H(privHash[256:512] || data)
		// "EdDSA avoids these issues by generating r = H(h_b, ..., h_2b-1, M), so that
		//  different messages will lead to different, hard-to-predict values of r."
		bignum256modm r;
		GenerateNonce(keyPair.privateKey(), buffersList, r);

		// R = rModQ * base point
		ge25519 ALIGN(16) R;
		ge25519_scalarmult_base_niels(&R, ge25519_niels_base_multiples, r);
		ge25519_pack(encodedR, &R);

		// h = H(encodedR || public || data)
		Hash512 hash_h;
		Sha512_Builder hasher_h;
		hasher_h.update({ { encodedR, Encoded_Size }, keyPair.publicKey() });
		hasher_h.update(buffersList);
		hasher_h.final(hash_h);

		bignum256modm h;
		expand256_modm(h, hash_h.data(), 64);

		// hash the private key to improve randomness
		Hash512 privHash;
		HashPrivateKey(keyPair.privateKey(), privHash);

		// a = fieldElement(privHash[0:256])
		privHash[0] &= 0xF8;
		privHash[31] &= 0x7F;
		privHash[31] |= 0x40;

		bignum256modm a;
		expand256_modm(a, privHash.data(), 32);

		// S = (r + h * a) mod group order
		bignum256modm S;
		mul256_modm(S, h, a);
		add256_modm(S, S, r);
		contract256_modm(encodedS, S);

		// signature is (encodedR, encodedS)

		// throw if encodedS is not less than the group order, don't fail in case encodedS == 0
		// (this should only throw if there is a bug in the signing code)
		CheckEncodedS(encodedS);

		SecureZero(privHash);
		SecureZero(r);
		SecureZero(a);
	}

	// endregion

	// region Verify

	bool Verify(const Key& publicKey, const RawBuffer& dataBuffer, const Signature& signature) {
		return Verify(publicKey, std::vector<RawBuffer>{ dataBuffer }, signature);
	}

	bool Verify(const Key& publicKey, const std::vector<RawBuffer>& buffers, const Signature& signature) {
		const uint8_t *RESTRICT encodedR = signature.data();
		const uint8_t *RESTRICT encodedS = signature.data() + Encoded_Size;

		// reject if not canonical
		if (!IsCanonicalS(encodedS))
			return false;

		// reject zero public key, which is known weak key
		if (Key() == publicKey)
			return false;

		// h = H(encodedR || public || data)
		Hash512 hash_h;
		Sha512_Builder hasher_h;
		hasher_h.update({ { encodedR, Encoded_Size }, publicKey });
		for (const auto& buffer : buffers)
			hasher_h.update(buffer);

		hasher_h.final(hash_h);

		bignum256modm h;
		expand256_modm(h, hash_h.data(), 64);

		// A = -pub
		ge25519 ALIGN(16) A;
		if (!UnpackNegativeAndCheckSubgroup(A, publicKey))
			return false;

		bignum256modm S;
		expand256_modm(S, encodedS, 32);

		// R = encodedS * B - h * A
		ge25519 ALIGN(16) R;
		ge25519_double_scalarmult_vartime(&R, &A, h, S);

		// compare calculated R to given R
		uint8_t checkr[Encoded_Size];
		ge25519_pack(checkr, &R);
		return 1 == ed25519_verify(encodedR, checkr, 32);
	}

	// endregion

	// region VerifyMulti

	namespace {
		std::pair<std::vector<bool>, bool> CheckForCanonicalFormAndNonzeroKeys(const SignatureInput* pSignatureInputs, size_t count) {
			// reject if not canonical or public key is zero
			auto aggregateResult = true;
			std::vector<bool> valid(count, true);
			for (auto i = 0u; i < count; ++i) {
				if (!IsCanonicalS(pSignatureInputs[i].Signature.data() + Encoded_Size) || Key() == pSignatureInputs[i].PublicKey) {
					aggregateResult = false;
					valid[i] = false;
				}
			}

			return std::make_pair(valid, aggregateResult);
		}

		bool VerifySingle(const SignatureInput* pSignatureInputs, size_t offset, size_t count, std::vector<bool>& valid) {
			bool aggregateResult = true;
			for (auto i = 0u; i < count; ++i) {
				valid[offset + i] = Verify(pSignatureInputs[i].PublicKey, pSignatureInputs[i].Buffers, pSignatureInputs[i].Signature);
				aggregateResult &= valid[offset + i];
			}

			return aggregateResult;
		}

		bool VerifyBatches(
				const RandomFiller& randomFiller,
				const SignatureInput* pSignatureInputs,
				size_t count,
				std::pair<std::vector<bool>, bool>& result,
				const predicate<size_t, size_t>& fallback) {
			size_t offset = 0;
			batch_heap ALIGN(16) batch;
			ge25519 ALIGN(16) p;
			bignum256modm* r_scalars;
			size_t batchSize;
			auto& aggregateResult = result.second;

			// because batch verification has some overhead like computing scalars, it is only faster when verifying more than 3 signatures
			while (count > 3) {
				batchSize = (count > max_batch_size) ? max_batch_size : count;

				// generate r (scalars[batchSize+1]..scalars[2*batchSize]
				// compute scalars[0] = ((r1s1 + r2s2 + ...))
				randomFiller(reinterpret_cast<uint8_t*>(batch.r), batchSize * 16);
				r_scalars = &batch.scalars[batchSize + 1];
				for (auto i = 0u; i < batchSize; ++i) {
					expand256_modm(r_scalars[i], batch.r[i], 16);
					expand256_modm(batch.scalars[i], pSignatureInputs[offset + i].Signature.data() + 32, 32);
					mul256_modm(batch.scalars[i], batch.scalars[i], r_scalars[i]);
					if (0u < i)
						add256_modm(batch.scalars[0], batch.scalars[0], batch.scalars[i]);
				}

				// compute scalars[1]..scalars[batchSize] as r[i]*H(R[i],A[i],m[i])
				for (auto i = 0u; i < batchSize; ++i) {
					Hash512 hash_h;
					Sha512_Builder hasher_h;
					const auto& signatureInput = pSignatureInputs[offset + i];
					hasher_h.update({ { signatureInput.Signature.data(), Encoded_Size }, signatureInput.PublicKey });
					for (const auto& buffer : signatureInput.Buffers)
						hasher_h.update(buffer);

					hasher_h.final(hash_h);

					expand256_modm(batch.scalars[i + 1], hash_h.data(), 64);
					mul256_modm(batch.scalars[i + 1], batch.scalars[i + 1], r_scalars[i]);
				}

				// compute points
				batch.points[0] = ge25519_basepoint;
				bool success = true;
				for (auto i = 0u; i < batchSize; ++i) {
					const auto& signatureInput = pSignatureInputs[offset + i];
					auto R = signatureInput.Signature.copyTo<Key>();
					success &= UnpackNegativeAndCheckSubgroup(batch.points[i + 1], signatureInput.PublicKey);
					success &= UnpackNegativeAndCheckSubgroup(batch.points[batchSize + i + 1], R);
					if (!success)
						break;
				}

				if (success) {
					ge25519_multi_scalarmult_vartime(&p, &batch, (batchSize * 2) + 1);
					success = ge25519_is_neutral_vartime(&p);
				}

				// fallback if batch verification failed
				if (!success && !fallback(offset, batchSize))
					return false;

				count -= batchSize;
				offset += batchSize;
			}

			aggregateResult &= VerifySingle(pSignatureInputs, offset, count, result.first);
			return aggregateResult;
		}
	}

	std::pair<std::vector<bool>, bool> VerifyMulti(
			const RandomFiller& randomFiller,
			const SignatureInput* pSignatureInputs,
			size_t count) {
		auto result = CheckForCanonicalFormAndNonzeroKeys(pSignatureInputs, count);
		VerifyBatches(randomFiller, pSignatureInputs, count, result, [&pSignatureInputs, &result](auto offset, auto batchSize) {
			result.second &= VerifySingle(pSignatureInputs, offset, batchSize, result.first);
			return true;
		});
		return result;
	}

	bool VerifyMultiShortCircuit(const RandomFiller& randomFiller, const SignatureInput* pSignatureInputs, size_t count) {
		auto result = CheckForCanonicalFormAndNonzeroKeys(pSignatureInputs, count);
		return result.second && VerifyBatches(randomFiller, pSignatureInputs, count, result, [](auto, auto) {
			return false;
		});
	}

	// endregion
}}
