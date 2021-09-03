import "transaction.cats"

# cosignature attached to an aggregate transaction
struct Cosignature
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MULTISIG_COSIGNATURE)

	inline Transaction

	# [__value__] multisig transaction hash
	#
	# [size] multisig transaction hash size
	multisig_transaction_hash = inline SizePrefixedHash256

	# [__value__] multisig account address
	#
	# [size] multisig account size
	multisig_account_address = inline SizePrefixedPublicKey

# binary layout for a multisig transaction
struct MultisigTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MULTISIG_COSIGNATURE)

	inline Transaction

	# inner transaction size
	inner_transaction_size = uint32

	# inner transaction
	inner_transaction = Transaction

	# number of attached cosignatures
	cosignatures_count = uint32

	# cosignatures
	cosignatures = array(Cosignature, cosignatures_count)
