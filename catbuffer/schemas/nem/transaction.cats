import "entity.cats"
import "transaction_type.cats"

# binary layout for a transaction
struct Transaction
	# transaction type
	type = TransactionType

	inline EntityBody

	# transaction fee
	fee = Amount

	# transaction deadline
	deadline = Timestamp
