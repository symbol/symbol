#include "CryptoUtils.h"
#include "Hashes.h"
#include "PrivateKey.h"
#include "SecureZero.h"

namespace catapult { namespace crypto {

	void HashPrivateKey(const PrivateKey& privateKey, Hash512& hash) {
#ifdef NIS1_COMPATIBLE_SIGNATURES
		Key reversedKey;
		std::reverse_copy(privateKey.cbegin(), privateKey.cend(), reversedKey.begin());
		Sha3_512(reversedKey, hash);
		SecureZero(reversedKey.data(), Key_Size);
#else
		Sha3_512({ privateKey.data(), privateKey.size() }, hash);
#endif
	}
}}
