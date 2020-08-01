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

#include "CryptoUtils.h"
#include "Hashes.h"
#include "PrivateKey.h"
#include "SecureZero.h"
#include <donna/catapult.h>

namespace catapult { namespace crypto {

	namespace {
		// region byte / byte array helpers

		bool IsZero(uint32_t b) {
			return 1 == (((b - 1) >> 8) & 1);
		}

		uint8_t IsNegative(int8_t b) {
			auto x = static_cast<uint8_t>(b);
			x = static_cast<uint8_t>(x >> 7);
			return x;
		}

		uint8_t IsEqual(int8_t b, int8_t c) {
			auto ub = static_cast<uint8_t>(b);
			auto uc = static_cast<uint8_t>(c);
			auto x = static_cast<uint8_t>(ub ^ uc);
			x = static_cast<uint8_t>(x - 1);
			x = static_cast<uint8_t>(x >> 7);
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
				e[i] = static_cast<int8_t>(e[i] + carry);
				carry = static_cast<int8_t>(e[i] + 8);
				carry = static_cast<int8_t>(carry >> 4);
				e[i] = static_cast<int8_t>(e[i] - carry * (1 << 4));
			}

			e[63] = static_cast<int8_t>(e[63] + carry);
		}

		// endregion

		// region field element helpers

		void SetZero(bignum25519 f) {
			std::memset(&f[0], 0, 5 * sizeof(uint64_t));
		}

		void SetOne(bignum25519 f) {
			f[0] = 1;
			std::memset(&f[1], 0, 4 * sizeof(uint64_t));
		}

		bool IsZeroScalar(const uint8_t* n, size_t nlen) {
			uint32_t a = 0;
			for (auto i = 0u; i < nlen; ++i)
				a |= n[i];

			return IsZero(a);
		}

		bool IsEqualScalar(const uint8_t* x, const uint8_t* y, size_t nlen) {
			uint32_t a = 0;
			for (auto i = 0u; i < nlen; ++i)
				a |= x[i] ^ y[i];

			return IsZero(a);
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

		void ScalarMultGroupOrder(ge25519& H, const ge25519& A) {
			// precompute table for A
			ge25519_pniels precomputedTable[8];
			PrecomputeTable(A, precomputedTable);

			// group order q represented by radix 16
			int8_t q[64] = {
				-3, -1, 4, -3, 6, -1, -3, 6, -6, 2, 3, 6, 2, 1, -8, 6,
				6, -3, -3, -6, -8, 0, 3, -6, -1, -2, -6, 0, -1, -2, 5, 1,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
			};

			// scalar multiplication
			ge25519_p1p1 R;
			ge25519 G;
			SetZero(H);
			for (auto i = 63; 0 <= i; --i) {
				if (0 != q[i]) {
					auto signbit = static_cast<uint8_t>((q[i] & 0xFF) >> 7);
					ge25519_pnielsadd_p1p1(&R, &H, &precomputedTable[abs(q[i]) - 1], signbit);
					ge25519_p1p1_to_full(&H, &R);
				}

				if (0 != i) {
					ge25519_double(&G, &H);
					ge25519_double(&H, &G);
					ge25519_double(&G, &H);
					ge25519_double(&H, &G);
				}
			}
		}

		// endregion
	}

	// publicKey is canonical if the y coordinate is smaller than 2^255 - 19
	bool IsCanonicalKey(const Key& publicKey) {
		// 0 != a if bits 8 through 254 of data are all set
		const auto* buffer = publicKey.data();
		uint32_t a = (buffer[31] & 0x7F) ^ 0x7F;
		for (auto i = 30u; i > 0; --i)
			a |= buffer[i] ^ 0xFF;

		a = (a - 1) >> 8;

		// 0 != b if data[0] < 256 - 19
		uint32_t b = (0xED - 1u - static_cast<uint32_t>(buffer[0])) >> 8;
		return 0 != 1 - (a & b & 1);
	}

	bool IsNeutralElement(const Key& publicKey) {
		const auto* buffer = publicKey.data();
		uint32_t c = static_cast<uint8_t>(buffer[0] ^ 0x01);
		for (auto i = 1u; i < 31; ++i)
			c |= buffer[i];

		c |= buffer[31] & 0x7F;

		return IsZero(c);
	}

	// multiply by the group order q and check if the result is the neutral element
	bool IsInMainSubgroup(const ge25519& A) {
		ge25519 R;
		ScalarMultGroupOrder(R, A);

		uint8_t contractedX[32];
		curve25519_contract(contractedX, R.x);
		uint8_t contractedY[32];
		curve25519_contract(contractedY, R.y);
		uint8_t contractedZ[32];
		curve25519_contract(contractedZ, R.z);
		return IsZeroScalar(contractedX, 32) && IsEqualScalar(contractedY, contractedZ, 32);
	}

	bool UnpackNegative(ge25519& A, const Key& publicKey) {
		return IsCanonicalKey(publicKey) && 0 != ge25519_unpack_negative_vartime(&A, publicKey.data());
	}

	bool UnpackNegativeAndCheckSubgroup(ge25519& A, const Key& publicKey) {
		return UnpackNegative(A, publicKey) && IsInMainSubgroup(A);
	}

	void HashPrivateKey(const PrivateKey& privateKey, Hash512& hash) {
		Sha512({ privateKey.data(), privateKey.size() }, hash);
	}

	void ExtractMultiplier(const PrivateKey& privateKey, ScalarMultiplier& multiplier) {
		Hash512 privHash;
		HashPrivateKey(privateKey, privHash);

		// fieldElement(privHash[0:256])
		privHash[0] &= 0xF8;
		privHash[31] &= 0x7F;
		privHash[31] |= 0x40;
		std::memcpy(multiplier, privHash.data(), Hash256::Size);
		SecureZero(privHash);
	}

	void GenerateNonce(const PrivateKey& privateKey, std::initializer_list<const RawBuffer> buffersList, bignum256modm_type& nonce) {
		Hash512 privHash;
		HashPrivateKey(privateKey, privHash);
		Hash512 hash;

		Sha512_Builder builder;
		builder.update({ privHash.data() + Hash512::Size / 2, Hash512::Size / 2 });
		builder.update(buffersList);
		builder.final(hash);
		expand256_modm(nonce, hash.data(), 64);

		SecureZero(privHash);
	}

	bool ScalarMult(const ScalarMultiplier& multiplier, const Key& publicKey, Key& sharedSecret) {
		// unpack public key
		ge25519 A;
		if (!UnpackNegativeAndCheckSubgroup(A, publicKey))
			return false;

		// negate A
		curve25519_neg(A.x, A.x);
		curve25519_neg(A.t, A.t);

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
		SecureZero(e);
		return !IsNeutralElement(sharedSecret);
	}
}}
