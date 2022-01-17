import "entity.cats"
import "transaction_type.cats"

# binary layout for a transaction
@initializes(version, TRANSACTION_VERSION)
@initializes(type, TRANSACTION_TYPE)
@discriminator(type, version)
abstract struct Transaction
	# transaction type
	type = TransactionType

	inline EntityBody

	# transaction fee
	fee = Amount

	# transaction deadline
	deadline = Timestamp

# binary layout for a non-verifiable transaction
@initializes(version, TRANSACTION_VERSION)
@initializes(type, TRANSACTION_TYPE)
@discriminator(type, version)
@is_size_implicit
abstract struct NonVerifiableTransaction
	# transaction type
	type = TransactionType

	inline NonVerifiableEntityBody

	# transaction fee
	fee = Amount

	# transaction deadline
	deadline = Timestamp
