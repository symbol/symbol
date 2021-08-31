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
	proof = array(uint8, proofSize)

# binary layout for a non-embedded secret proof transaction
struct SecretProofTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, secret_proof)

	inline Transaction
	inline SecretProofTransactionBody

# binary layout for an embedded secret proof transaction
struct EmbeddedSecretProofTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, secret_proof)

	inline EmbeddedTransaction
	inline SecretProofTransactionBody
