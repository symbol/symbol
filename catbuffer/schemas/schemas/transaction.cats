import "entity.cats"

# binary layout for a transaction
struct Transaction
	inline SizePrefixedEntity
	inline VerifiableEntity
	inline EntityBody

	# transaction fee
	fee = Amount

	# transaction deadline
	deadline = Timestamp

# binary layout for an embedded transaction
struct EmbeddedTransaction
	inline SizePrefixedEntity
	inline EntityBody
