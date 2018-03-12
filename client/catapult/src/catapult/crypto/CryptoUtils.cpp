#include "CryptoUtils.h"
#include "Hashes.h"
#include "PrivateKey.h"
#include "SecureZero.h"
#ifdef SIGNATURE_SCHEME_NIS1
#include <algorithm>
#endif

namespace catapult { namespace crypto {

	void HashPrivateKey(const PrivateKey& privateKey, Hash512& hash) {
#ifdef SIGNATURE_SCHEME_NIS1
		Key reversedKey;
		std::reverse_copy(privateKey.begin(), privateKey.end(), reversedKey.begin());
		Sha3_512(reversedKey, hash);
		SecureZero(reversedKey.data(), Key_Size);
#else
		Sha3_512({ privateKey.data(), privateKey.size() }, hash);
#endif
	}
}}
