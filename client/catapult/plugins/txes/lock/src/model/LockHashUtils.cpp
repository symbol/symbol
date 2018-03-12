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
