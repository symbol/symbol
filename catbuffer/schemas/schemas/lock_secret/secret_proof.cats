import "lock_secret/lock_secret_types.cats"
import "transaction.cats"

# binary layout for a secret proof transaction
struct SecretProofTransactionBody
	# locked mosaic recipient address
	recipientAddress = UnresolvedAddress

	# secret
	secret = Hash256

	# proof size in bytes
	proofSize = uint16

	# hash algorithm
	hashAlgorithm = LockHashAlgorithm

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
