# lock secret hash algorithm
enum LockHashAlgorithm : uint8
	# input is hashed using sha-3 512
	sha3_512 = 0

	# input is hashed using keccak 256
	keccak_256 = 1

	# input is hashed twice: first with sha-256 and then with ripemd-160 (bitcoin's OP_HASH160)
	hash_160 = 2

	# input is hashed twice with sha-256 (bitcoin's OP_HASH256)
	hash_256 = 3
