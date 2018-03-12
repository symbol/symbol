#pragma once
#include <stdint.h>

namespace catapult { namespace model {

	/// Lock secret hash algorithm.
	enum class LockHashAlgorithm : uint8_t {
		/// The input is hashed using Sha-3.
		Op_Sha3,
		/// The input is hashed using Keccak.
		Op_Keccak,
		/// The input is hashed twice: first with SHA-256 and then with RIPEMD-160.
		Op_Hash_160,
		/// The input is hashed twice with SHA-256.
		Op_Hash_256
	};
}}
