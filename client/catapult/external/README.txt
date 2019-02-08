ref10 - public domain, implementation from supercop
http://bench.cr.yp.to/supercop/supercop-20190110.tar.xz
crypo_sign\ed25519\ref10


sha3 - public domain CC0,
Common\brg_endian.h - is copyrighted by Brian Gladman and comes with a BSD 3-clause license
implementation from Keccak repository:
https://github.com/gvanas/KeccakCodePackage 58b20ec99f8a891913d8cf0ea350d05b6fb3ae41

lib\high\Keccak\FIPS202\KeccakHash.[ch]
lib\high\Keccak\KeccakSponge.inc
lib\high\Keccak\KeccakSponge-common.h
lib\high\Keccak\KeccakSpongeWidth1600.[ch]
lib\low\common\SnP-Relaned.h
lib\low\KeccakP-1600\Optimized\KeccakP-1600-unrolling.macros
lib\low\KeccakP-1600\Optimized\KeccakP-1600-64.macros
lib\low\KeccakP-1600\Optimized64\KeccakP-1600-opt64.c
lib\low\KeccakP-1600\Optimized64\KeccakP-1600-SnP.h
lib\low\KeccakP-1600\Optimized64\ufull\KeccakP-1600-opt64-config.h


ripemd160 - MIT BSD, implementation of the ripemd160 algorithm:
https://github.com/rustyrussell/ccan.git 7082f7d0e81911acb26787949c251dfb298cbdd8
ccan/crypto/ripemd160/
ccan/compiler/compiler.h
ccan/endian/endian.h

vectors:
https://homes.esat.kuleuven.be/~bosselae/ripemd160.html


sha256 - ISC, libsodium
https://github.com/jedisct1/libsodium.git 7d418f8203897a967cce3dc0dcf8925f894e3428 (stable branch)

as is:
private/common.h
crypto_hash_sha256.h
hash_sha256.c
hash_sha256_cp.c

modified:
export.h
utils.[ch]
