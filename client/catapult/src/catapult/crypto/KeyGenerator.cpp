#include "KeyGenerator.h"
#include "CryptoUtils.h"
#include "PrivateKey.h"

extern "C" {
#include <ref10/ge.h>
}

namespace catapult { namespace crypto {

	void ExtractPublicKeyFromPrivateKey(const PrivateKey& privateKey, Key& publicKey) {
		Hash512 h;
		ge_p3 A;

		HashPrivateKey(privateKey, h);

		h[0] &= 0xF8;
		h[31] &= 0x7F;
		h[31] |= 0x40;

		ge_scalarmult_base(&A, h.data());
		ge_p3_tobytes(publicKey.data(), &A);
	}
}}
