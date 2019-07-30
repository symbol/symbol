# enumeration of lock hash algorithms
enum LockHashAlgorithm : uint8
	# input is hashed using sha-3 256
	sha3_256 = 0x00

	# input is hashed using keccak 256
	keccak_256 = 0x01

	# input is hashed twice: first with sha-256 and then with ripemd-160 (bitcoin's OP_HASH160)
	hash_160 = 0x02

	# input is hashed twice with sha-256 (bitcoin's OP_HASH256)
	hash_256 = 0x03
