/*
	a custom hash must have a 512bit digest and implement:

	struct ed25519_hash_context;

	void ed25519_hash_init(ed25519_hash_context *ctx);
	void ed25519_hash_update(ed25519_hash_context *ctx, const uint8_t *in, size_t inlen);
	void ed25519_hash_final(ed25519_hash_context *ctx, uint8_t *hash);
	void ed25519_hash(uint8_t *hash, const uint8_t *in, size_t inlen);
*/

#include "../sha3/KeccakHash.h"

typedef Keccak_HashInstance ed25519_hash_context;

void ed25519_hash_init(ed25519_hash_context *ctx) {
	Keccak_HashInitialize_SHA3_512(ctx);
}

void ed25519_hash_update(ed25519_hash_context *ctx, const uint8_t *in, size_t inlen) {
	Keccak_HashUpdate(ctx, in, inlen * 8);
}

void ed25519_hash_final(ed25519_hash_context *ctx, uint8_t *hash) {
#ifdef SIGNATURE_SCHEME_KECCAK
	Keccak_HashSqueeze(ctx, hash, 512);
#else
	Keccak_HashFinal(ctx, hash);
#endif
}

void ed25519_hash(uint8_t *hash, const uint8_t *in, size_t inlen) {
	ed25519_hash_context ctx;
	ed25519_hash_init(&ctx);
	ed25519_hash_update(&ctx, in, inlen);
	ed25519_hash_final(&ctx, hash);
}

