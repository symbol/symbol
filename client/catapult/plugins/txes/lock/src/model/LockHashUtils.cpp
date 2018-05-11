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

#include "LockHashUtils.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/utils/Casting.h"
#include "catapult/exceptions.h"

namespace catapult { namespace model {

	Hash512 CalculateHash(LockHashAlgorithm hashAlgorithm, const RawBuffer& data) {
		// zero initialize
		// note: in case of shorter hashes pad with 0s
		Hash512 hash{};
		switch (hashAlgorithm) {
		case LockHashAlgorithm::Op_Sha3:
			crypto::Sha3_512(data, hash);
			break;
		case LockHashAlgorithm::Op_Keccak:
		case LockHashAlgorithm::Op_Hash_160:
		case LockHashAlgorithm::Op_Hash_256:
			CATAPULT_THROW_INVALID_ARGUMENT_1("invalid hash algorithm", utils::to_underlying_type(hashAlgorithm));
		}

		return hash;
	}
}}
