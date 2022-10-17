import "transaction.cats"

# cosignature attached to an aggregate transaction
@is_size_implicit
struct Cosignature
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
struct SizePrefixedCosignature
	# cosignature size
	cosignature_size = sizeof(uint32, cosignature)

	# cosignature
	cosignature = Cosignature

# binary layout for a multisig transaction (V1, latest)
struct MultisigTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MULTISIG_TRANSACTION)

	inline Transaction

	# inner transaction size
	inner_transaction_size = sizeof(uint32, inner_transaction)

	# inner transaction
	inner_transaction = NonVerifiableTransaction

	# number of attached cosignatures
	cosignatures_count = uint32

	# cosignatures
	cosignatures = array(SizePrefixedCosignature, cosignatures_count)
