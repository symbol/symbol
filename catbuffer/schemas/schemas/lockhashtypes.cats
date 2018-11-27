# lock secret hash algorithm
enum LockHashAlgorithm : uint8
	# input is hashed using sha-3
	sha3 = 0
	# input is hashed using keccak
	keccak = 1
	# input is hashed twice: first with sha-256 and then with ripemd-160 (bitcoin's OP_HASH160)
	hash160 = 2
	# input is hashed twice with sha-256 (bitcoin's OP_HASH256)
	hash256 = 3
