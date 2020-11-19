/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "LockHashUtils.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/utils/Casting.h"
#include "catapult/exceptions.h"

namespace catapult { namespace model {

	namespace {
		template<typename THasher>
		Hash256 CalculateHash256(THasher hasher, const RawBuffer& data) {
			Hash256 hash;
			hasher(data, hash);
			return hash;
		}

		template<typename THashType, typename THasher>
		Hash256 CalculateHash(THasher hasher, const RawBuffer& data) {
			THashType hash;
			hasher(data, hash);
			return hash.template copyTo<Hash256>();
		}
	}

	Hash256 CalculateHash(LockHashAlgorithm hashAlgorithm, const RawBuffer& data) {
		switch (hashAlgorithm) {
		case LockHashAlgorithm::Op_Sha3_256:
			return CalculateHash256(crypto::Sha3_256, data);

		case LockHashAlgorithm::Op_Hash_160:
			return CalculateHash<Hash160>(crypto::Bitcoin160, data);

		case LockHashAlgorithm::Op_Hash_256:
			return CalculateHash256(crypto::Sha256Double, data);
		}

		CATAPULT_THROW_INVALID_ARGUMENT_1("invalid hash algorithm", utils::to_underlying_type(hashAlgorithm));
	}

	Hash256 CalculateSecretLockInfoHash(const Hash256& secret, const Address& recipient) {
		Hash256 hash;
		crypto::Sha3_256_Builder builder;
		builder.update({ secret, recipient });
		builder.final(hash);
		return hash;
	}
}}
