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

extern "C" {
#include <ref10/fe.h>
#include <ref10/ge.h>
#include <ref10/sc.h>
}

namespace catapult { namespace crypto {

	SharedKey DeriveSharedKey(const KeyPair& keyPair, const Key& otherPublicKey, const Salt& salt) {
		// prepare for scalar multiply
		Hash512 privHash;
		HashPrivateKey(keyPair.privateKey(), privHash);

		// fieldElement(privHash[0:256])
		privHash[0] &= 0xF8;
		privHash[31] &= 0x7F;
		privHash[31] |= 0x40;

		// A = public key
		ge_p3 A;
		ge_frombytes_negate_vartime(&A, otherPublicKey.data());

		// R = privHash * A
		Hash512 zero{};
		ge_p2 R;
		ge_double_scalarmult_vartime(&R, privHash.data(), &A, zero.data());
		fe_neg(R.X, R.X);

		// store result
		Hash256 saltedResult;
		ge_tobytes(saltedResult.data(), &R);

		// salt and hash
		for (auto i = 0u; i < saltedResult.size(); ++i)
			saltedResult[i] ^= salt[i];

		Hash256 hash;
#ifdef SIGNATURE_SCHEME_NIS1
		Keccak_256(saltedResult, hash);
#else
		Sha3_256(saltedResult, hash);
#endif

		SharedKey shared;
		std::memcpy(shared.data(), hash.data(), SharedKey::Size);
		return shared;
	}
}}
