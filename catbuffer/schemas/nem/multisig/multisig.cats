import "transaction.cats"

# shared content between V1 verifiable and non-verifiable cosignature transactions
struct CosignatureV1Body
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MULTISIG_COSIGNATURE)

	# other transaction hash outer size
	other_transaction_hash_outer_size = make_reserved(uint32, 36)

	# [__value__] other transaction hash
	#
	# [size] other transaction hash size
	other_transaction_hash = inline SizePrefixedHash256

	# [__value__] multisig account address
	#
	# [size] multisig account address size
	multisig_account_address = inline SizePrefixedAddress

# binary layout for a cosignature transaction (V1, latest)
@is_size_implicit
struct CosignatureV1
	inline Transaction
	inline CosignatureV1Body

# binary layout for a non-verifiable cosignature transaction (V1, latest)
struct NonVerifiableCosignatureV1
	inline NonVerifiableTransaction
	inline CosignatureV1Body

# cosignature attached to a multisig transaction with prefixed size
struct SizePrefixedCosignatureV1
	# cosignature size
	cosignature_size = sizeof(uint32, cosignature)

	# cosignature
	cosignature = CosignatureV1

# shared content between V1 verifiable and non-verifiable multisig transactions
inline struct MultisigTransactionV1Body
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MULTISIG)

	# inner transaction size
	inner_transaction_size = sizeof(uint32, inner_transaction)

	# inner transaction
	inner_transaction = NonVerifiableTransaction

# binary layout for a multisig transaction (V1, latest)
struct MultisigTransactionV1
	inline Transaction
	inline MultisigTransactionV1Body

	# number of attached cosignatures
	cosignatures_count = uint32

	# cosignatures
	cosignatures = array(SizePrefixedCosignatureV1, cosignatures_count)

# binary layout for a non-verifiable multisig transaction (V1, latest)
struct NonVerifiableMultisigTransactionV1
	inline NonVerifiableTransaction
	inline MultisigTransactionV1Body

