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

#include "SharedKey.h"
#include "CryptoUtils.h"
#include "Hashes.h"
#include <cstring>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wcast-align"
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wdocumentation"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4324) /* ed25519 structs use __declspec(align()) */
#pragma warning(disable : 4388) /* signed/unsigned mismatch */
#pragma warning(disable : 4505) /* unreferenced local function has been removed */
#endif

extern "C" {
#include <donna/ed25519-donna.h>
#include <donna/modm-donna-64bit.h>
}

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace catapult { namespace crypto {

	SharedKey Hkdf_Hmac_Sha256_32(const SharedSecret& sharedSecret) {
		Hash256 salt;
		Hash256 pseudoRandomKey;
		Hmac_Sha256(salt, sharedSecret, pseudoRandomKey);

		// specialized for single repetition, last byte contains counter value
		constexpr auto Buffer_Length = 8 + 1;
		std::array<uint8_t, Buffer_Length> buffer{ { 0x63, 0x61, 0x74, 0x61, 0x70, 0x75, 0x6C, 0x74, 0x01 } };

		Hash256 outputKeyingMaterial;
		Hmac_Sha256(pseudoRandomKey, buffer, outputKeyingMaterial);

		SharedKey sharedKey;
		std::memcpy(sharedKey.data(), outputKeyingMaterial.data(), outputKeyingMaterial.size());
		return sharedKey;
	}

	namespace {
		// region byte / byte array helpers

		uint8_t IsNegative(int8_t b) {
			auto x = static_cast<uint8_t>(b);
			x >>= 7;
			return x;
		}

		uint8_t IsEqual(int8_t b, int8_t c) {
			auto ub = static_cast<uint8_t>(b);
			auto uc = static_cast<uint8_t>(c);
			auto x = static_cast<uint8_t>(ub ^ uc);
			--x;
			x >>= 7;
			return x;
		}

		// converts to radix 16 where -8 <= e[i] <= 7 for i = 0, ..., 63
		// prerequisite: a[31] <= 127
		void ToRadix16(int8_t (&e)[64], const uint8_t (&a)[32]) {
			for (auto i = 0u; i < 32; ++i) {
				e[2 * i + 0] = (a[i] >> 0) & 15;
				e[2 * i + 1] = (a[i] >> 4) & 15;
			}

			int8_t carry = 0;
			for (auto i = 0u; i < 63; ++i) {
				e[i] += carry;
				carry = e[i] + 8;
				carry >>= 4;
				e[i] -= carry * (static_cast<int8_t>(1) << 4);
			}

			e[63] += carry;
		}

		// endregion

		// region field element helpers

		// f = 0
		void SetZero(bignum25519 f) {
			std::memset(&f[0], 0, 5 * sizeof(uint64_t));
		}

		// f = 1
		void SetOne(bignum25519 f) {
			f[0] = 1;
			std::memset(&f[1], 0, 4 * sizeof(uint64_t));
		}

		// f = flag ? g : f
		// prerequisite: flag must be 0 or 1
		void ConditionalMove(bignum25519 f, const bignum25519 g, uint8_t flag) {
			uint64_t nb = static_cast<uint64_t>(flag - 1);
			uint64_t b = ~nb;
			f[0] = (f[0] & nb) | (g[0] & b);
			f[1] = (f[1] & nb) | (g[1] & b);
			f[2] = (f[2] & nb) | (g[2] & b);
			f[3] = (f[3] & nb) | (g[3] & b);
			f[4] = (f[4] & nb) | (g[4] & b);
		}

		// endregion

		// region group element helpers

		// A = neutral element
		void SetZero(ge25519& A) {
			SetZero(A.x);
			SetOne(A.y);
			SetOne(A.z);
			SetZero(A.t);
		}

		// A = neutral element
		void SetZero(ge25519_pniels& A) {
			SetOne(A.xaddy);
			SetOne(A.ysubx);
			SetOne(A.z);
			SetZero(A.t2d);
		}

		// A = flag ? B : A
		// prerequisite: flag must be 0 or 1
		void ConditionalMove(ge25519_pniels& A, const ge25519_pniels& B, uint8_t flag) {
			ConditionalMove(A.xaddy, B.xaddy, flag);
			ConditionalMove(A.ysubx, B.ysubx, flag);
			ConditionalMove(A.z, B.z, flag);
			ConditionalMove(A.t2d, B.t2d, flag);
		}

		// T = b * A, pTable holds group elements { A, 2 * A, 3 * A, 4 * A, 5 * A, 6 * A, 7 * A, 8 * A }
		// prerequisite: -8 <= b <= 7
		void Select(ge25519_pniels& T, const ge25519_pniels (&table)[8], int8_t b) {
			auto isNegative = IsNegative(b);
			auto ub = static_cast<uint8_t>(b);
			auto bAbs = static_cast<int8_t>(ub - (((-isNegative) & ub) << 1));

			SetZero(T);
			ConditionalMove(T, table[0], IsEqual(bAbs, 1));
			ConditionalMove(T, table[1], IsEqual(bAbs, 2));
			ConditionalMove(T, table[2], IsEqual(bAbs, 3));
			ConditionalMove(T, table[3], IsEqual(bAbs, 4));
			ConditionalMove(T, table[4], IsEqual(bAbs, 5));
			ConditionalMove(T, table[5], IsEqual(bAbs, 6));
			ConditionalMove(T, table[6], IsEqual(bAbs, 7));
			ConditionalMove(T, table[7], IsEqual(bAbs, 8));

			ge25519_pniels minusT;
			curve25519_copy(minusT.xaddy, T.ysubx);
			curve25519_copy(minusT.ysubx, T.xaddy);
			curve25519_copy(minusT.z, T.z);
			curve25519_neg(minusT.t2d, T.t2d);
			ConditionalMove(T, minusT, isNegative);
		}

		// pTable = { A, 2 * A, 3 * A, 4 * A, 5 * A, 6 * A, 7 * A, 8 * A }
		void PrecomputeTable(const ge25519& A, ge25519_pniels (&table)[8]) {
			// pTable[0] = A
			ge25519_full_to_pniels(&table[0], &A);

			// pTable[1] = 2 * A
			ge25519 A2;
			ge25519_double(&A2, &A);
			ge25519_full_to_pniels(&table[1], &A2);

			// pTable[2] = 3 * A
			ge25519_pnielsadd(&table[2], &A, &table[1]);

			// pTable[3] = 4 * A
			ge25519 A4;
			ge25519_double(&A4, &A2);
			ge25519_full_to_pniels(&table[3], &A4);

			// pTable[4] = 5 * A
			ge25519_pnielsadd(&table[4], &A, &table[3]);

			// pTable[5] = 6 * A
			ge25519_pnielsadd(&table[5], &A, &table[4]);

			// pTable[6] = 7 * A
			ge25519_pnielsadd(&table[6], &A, &table[5]);

			// pTable[7] = 8 * A
			ge25519 A8;
			ge25519_double(&A8, &A4);
			ge25519_full_to_pniels(&table[7], &A8);
		}

		// endregion

		bool ScalarMult(SharedSecret& sharedSecret, const uint8_t (&multiplier)[32], const Key& publicKey) {
			// unpack public key
			ge25519 A;
			if (!ge25519_unpack_negative_vartime(&A, publicKey.data()))
				return false;

			// negate A
			ge25519_p1p1 B;
			ge25519_pniels C;
			ge25519 D;
			ge25519_full_to_pniels(&C, &A);
			SetZero(D);
			ge25519_pnielsadd_p1p1(&B, &D, &C, 1);
			ge25519_p1p1_to_full(&A, &B);

			// precompute table for A
			ge25519_pniels precomputedTable[8];
			PrecomputeTable(A, precomputedTable);

			// convert multiplier to radix 16
			int8_t e[64];
			ToRadix16(e, multiplier);

			// scalar multiplication
			ge25519_pniels T;
			ge25519_p1p1 R;
			ge25519 G, H;
			SetZero(H);
			for (auto i = 63u; i != 0; --i) {
				Select(T, precomputedTable, e[i]);
				ge25519_pnielsadd_p1p1(&R, &H, &T, 0);

				ge25519_p1p1_to_full(&H, &R);
				ge25519_double(&G, &H);
				ge25519_double(&H, &G);
				ge25519_double(&G, &H);
				ge25519_double(&H, &G);
			}

			Select(T, precomputedTable, e[0]);
			ge25519_pnielsadd_p1p1(&R, &H, &T, 0);
			ge25519_p1p1_to_full(&H, &R);
			ge25519_pack(sharedSecret.data(), &H);
			return true;
		}
	}

	SharedKey DeriveSharedKey(const KeyPair& keyPair, const Key& otherPublicKey) {
		Hash512 privHash;
		HashPrivateKey(keyPair.privateKey(), privHash);

		// fieldElement(privHash[0:256])
		privHash[0] &= 0xF8;
		privHash[31] &= 0x7F;
		privHash[31] |= 0x40;
		uint8_t multiplier[32];
		std::memcpy(multiplier, privHash.data(), Hash256::Size);

		SharedSecret sharedSecret;
		if (!ScalarMult(sharedSecret, multiplier, otherPublicKey))
			return SharedKey();

		return Hkdf_Hmac_Sha256_32(sharedSecret);
	}
}}
