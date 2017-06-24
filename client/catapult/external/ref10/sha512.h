
#define crypto_hash_sha512_BYTES 64

#if __cplusplus
extern "C" {
#endif

	int crypto_hash_sha512(unsigned char *out, const unsigned char *in, unsigned long long inlen);

#if __cplusplus
}
#endif
