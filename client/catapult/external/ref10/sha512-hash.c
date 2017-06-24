#include "sha512.h"

#include "../sha3/KeccakHash.h"

int crypto_hash_sha512(unsigned char *out, const unsigned char *in, unsigned long long inlen)
{
	Keccak_HashInstance _hctx, *hctx = &_hctx;
	Keccak_HashInitialize_SHA3_512(hctx);
	Keccak_HashUpdate(hctx, in, inlen * 8);
#ifdef NIS1_COMPATIBLE_SIGNATURES
	Keccak_HashSqueeze(hctx, out, 512);
#else
	Keccak_HashFinal(hctx, out);
#endif
	return 0;
}
