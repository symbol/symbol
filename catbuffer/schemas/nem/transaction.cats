import "entity.cats"
import "transaction_type.cats"

# binary layout for a transaction
@initializes(version, TRANSACTION_VERSION)
@initializes(type, TRANSACTION_TYPE)
@discriminator(type)
abstract struct Transaction
	# transaction type
	type = TransactionType

	inline EntityBody

	# transaction fee
	fee = Amount

	# transaction deadline
	deadline = Timestamp
