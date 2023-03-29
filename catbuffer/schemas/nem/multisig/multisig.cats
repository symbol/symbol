import "transaction.cats"

# cosignature attached to an aggregate transaction
@is_size_implicit
struct CosignatureV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MULTISIG_COSIGNATURE)

	inline Transaction

	# multisig transaction hash outer size
	multisig_transaction_hash_outer_size = make_reserved(uint32, 36)

	# [__value__] multisig transaction hash
	#
	# [size] multisig transaction hash size
	multisig_transaction_hash = inline SizePrefixedHash256

	# [__value__] multisig account address
	#
	# [size] multisig account address size
	multisig_account_address = inline SizePrefixedAddress

# cosignature attached to an aggregate transaction with prefixed size
struct SizePrefixedCosignatureV1
	# cosignature size
	cosignature_size = sizeof(uint32, cosignature)

	# cosignature
	cosignature = CosignatureV1

# shared content between all verifiable and non-verifiable multisig transactions
inline struct MultisigTransactionBody
	TRANSACTION_TYPE = make_const(TransactionType, MULTISIG_TRANSACTION)

	# inner transaction size
	inner_transaction_size = sizeof(uint32, inner_transaction)

	# inner transaction
	inner_transaction = NonVerifiableTransaction

# shared content between all verifiable multisig transactions
inline struct MultisigTransactionVerifiableBody
	inline MultisigTransactionBody

	# number of attached cosignatures
	cosignatures_count = uint32

	# cosignatures
	cosignatures = array(SizePrefixedCosignatureV1, cosignatures_count)

# binary layout for a multisig transaction (V1)
struct MultisigTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)

	inline Transaction
	inline MultisigTransactionVerifiableBody

# binary layout for a non-verifiable multisig transaction (V1)
struct NonVerifiableMultisigTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)

	inline NonVerifiableTransaction
	inline MultisigTransactionBody

# binary layout for a multisig transaction (V2, latest)
struct MultisigTransactionV2
	TRANSACTION_VERSION = make_const(uint8, 2)

	inline Transaction
	inline MultisigTransactionVerifiableBody

# binary layout for a non-verifiable multisig transaction (V2, latest)
struct NonVerifiableMultisigTransactionV2
	TRANSACTION_VERSION = make_const(uint8, 2)

	inline NonVerifiableTransaction
	inline MultisigTransactionBody
