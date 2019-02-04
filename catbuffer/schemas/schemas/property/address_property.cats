import "property/property_types.cats"
import "transaction.cats"

# account properties address modification
struct AddressPropertyModification
	inline PropertyModification
	value = Address

# binary layout for an account properties address transaction
struct AddressPropertyTransactionBody
	# property type
	propertyType = PropertyType

	# number of modifications
	modificationsCount = uint8

	# property modifications
	modifications = array(AddressPropertyModification, modificationsCount)

# binary layout for a non-embedded account properties address transaction
struct AddressPropertyTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4151

	inline Transaction
	inline AddressPropertyTransactionBody

# binary layout for an embedded account properties address transaction
struct EmbeddedAddressPropertyTransaction
	inline EmbeddedTransaction
	inline AddressPropertyTransactionBody
