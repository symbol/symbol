# enumeration of lock hash algorithms
enum LockHashAlgorithm : uint8
	# input is hashed using sha-3 256
	SHA3_256 = 0x00

	# input is hashed twice: first with sha-256 and then with ripemd-160 (bitcoin's OP_HASH160)
	HASH_160 = 0x01

	# input is hashed twice with sha-256 (bitcoin's OP_HASH256)
	HASH_256 = 0x02
