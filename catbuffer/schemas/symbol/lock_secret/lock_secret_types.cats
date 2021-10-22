# Enumeration of lock hash algorithms.
enum LockHashAlgorithm : uint8
	# Input is hashed using [SHA-3 256](https://en.wikipedia.org/wiki/SHA-3).
	SHA3_256 = 0x00

	# Input is hashed twice: first with [SHA-256](https://en.wikipedia.org/wiki/SHA-2) and then with [RIPEMD-160](https://en.wikipedia.org/wiki/RIPEMD) (bitcoin's OP_HASH160).
	HASH_160 = 0x01

	# Input is hashed twice with [SHA-256](https://en.wikipedia.org/wiki/SHA-2) (bitcoin's OP_HASH256).
	HASH_256 = 0x02
