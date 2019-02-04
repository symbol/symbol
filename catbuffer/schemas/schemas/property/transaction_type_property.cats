import "property/property_types.cats"
import "transaction.cats"

# account properties transaction type modification
struct TransactionTypePropertyModification
	inline PropertyModification
	value = EntityType

# binary layout for an account properties transaction type transaction
struct TransactionTypePropertyTransactionBody
	# property type
	propertyType = PropertyType

	# number of modifications
	modificationsCount = uint8

	# property modifications
	modifications = array(TransactionTypePropertyModification, modificationsCount)

# binary layout for a non-embedded account properties transaction type transaction
struct TransactionTypePropertyTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4153

	inline Transaction
	inline TransactionTypePropertyTransactionBody

# binary layout for an embedded account properties transaction type transaction
struct EmbeddedTransactionTypePropertyTransaction
	inline EmbeddedTransaction
	inline TransactionTypePropertyTransactionBody
