import "transaction.cats"

# property types
enum PropertyType : uint8
	# property type is an address
	address = 0x01

	# property type is a mosaic id
	mosaicId = 0x02

	# property type is a transaction type
	transactionType = 0x04

	# property type sentinel
	sentinel = 0x05

	# property is interpreted as blocking operation
	block = 0x80

# property modification type
enum PropertyModificationType : uint8
	# add property value
	add = 0x00

	# remove property value
	del = 0x01

# account properties basic modification
struct PropertyModification
	modificationType = PropertyModificationType

# account properties address modification
struct AddressPropertyModification
	inline PropertyModification
	value = Address

# account properties mosaic modification
struct MosaicPropertyModification
	inline PropertyModification
	value = MosaicId

# account properties transaction type modification
struct TransactionTypePropertyModification
	inline PropertyModification
	value = EntityType

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

# binary layout for an account properties mosaic transaction
struct MosaicPropertyTransactionBody
	# property type
	propertyType = PropertyType

	# number of modifications
	modificationsCount = uint8

	# property modifications
	modifications = array(MosaicPropertyModification, modificationsCount)

# binary layout for a non-embedded account properties mosaic transaction
struct MosaicPropertyTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4152

	inline Transaction
	inline MosaicPropertyTransactionBody

# binary layout for an embedded account properties mosaic transaction
struct EmbeddedMosaicPropertyTransaction
	inline EmbeddedTransaction
	inline MosaicPropertyTransactionBody

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

