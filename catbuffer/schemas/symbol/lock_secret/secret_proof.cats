import "lock_secret/lock_secret_types.cats"
import "transaction.cats"

# Shared content between SecretProofTransaction and EmbeddedSecretProofTransaction.
inline struct SecretProofTransactionBody
	# Address that receives the funds once unlocked.
	recipient_address = UnresolvedAddress

	# Hashed proof.
	secret = Hash256

	# Proof size in bytes
	proof_size = uint16

	# Algorithm used to hash the proof.
	hash_algorithm = LockHashAlgorithm

	# Original random set of bytes that were hashed.
	proof = array(uint8, proof_size)

# Conclude a token swap between different chains (V1, latest).
#
# Use a SecretProofTransaction to unlock the funds locked by a SecretLockTransaction.
#
# The transaction must prove knowing the *proof* that unlocks the mosaics.
struct SecretProofTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, SECRET_PROOF)

	inline Transaction
	inline SecretProofTransactionBody

# Embedded version of SecretProofTransaction (V1, latest).
struct EmbeddedSecretProofTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, SECRET_PROOF)

	inline EmbeddedTransaction
	inline SecretProofTransactionBody
