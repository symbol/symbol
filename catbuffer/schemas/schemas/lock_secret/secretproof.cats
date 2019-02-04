import "lock_secret/lockhashalgorithm.cats"
import "transaction.cats"

# binary layout for a secret proof transaction
struct SecretProofTransactionBody
	# hash algorithm
	hashAlgorithm = LockHashAlgorithm

	# secret
	secret = Hash512

	# proof size in bytes
	proofSize = uint16

	# proof data
	proof = array(byte, proofSize)

# binary layout for a non-embedded secret proof transaction
struct SecretProofTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4252

	inline Transaction
	inline SecretProofTransactionBody

# binary layout for an embedded secret proof transaction
struct EmbeddedSecretProofTransaction
	inline EmbeddedTransaction
	inline SecretProofTransactionBody
