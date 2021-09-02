import "lock_secret/lock_secret_types.cats"
import "transaction.cats"

# binary layout for a secret proof transaction
struct SecretProofTransactionBody
	# locked mosaic recipient address
	recipient_address = UnresolvedAddress

	# secret
	secret = Hash256

	# proof size in bytes
	proof_size = uint16

	# hash algorithm
	hash_algorithm = LockHashAlgorithm

	# proof data
	proof = array(uint8, proof_size)

# binary layout for a non-embedded secret proof transaction
struct SecretProofTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, SECRET_PROOF)

	inline Transaction
	inline SecretProofTransactionBody

# binary layout for an embedded secret proof transaction
struct EmbeddedSecretProofTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, SECRET_PROOF)

	inline EmbeddedTransaction
	inline SecretProofTransactionBody
