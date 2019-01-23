import "transaction.cats"

# Property types.
enum PropertyType : uint8
	# Property type is an address.
	address = 0x01

	# Property type is a mosaic id.
	mosaicId = 0x02

	# Property type is a transaction type.
	transactionType = 0x04

	# Property type sentinel.
	sentinel = 0x05

	# Property is interpreted as blocking operation.
	block = 0x80

# Property modification type.
enum PropertyModificationType : uint8
	# Add property value.
	add = 0x00

	# Remove property value.
	del = 0x01

struct AccountPropertiesModification
	modificationType = PropertyModificationType

struct AccountPropertiesTransactionBody
	inline Transaction
	propertyType = PropertyType

struct AddressModification
	inline AccountPropertiesModification
	value = Address

struct MosaicModification
	inline AccountPropertiesModification
	value = MosaicId

struct EntityTypeModification
	inline AccountPropertiesModification
	value = EntityType

struct AccountPropertiesAddressTransaction
	inline AccountPropertiesTransactionBody
	modificationsCount = uint8
	modifications = array(AddressModification, modificationsCount)

struct AccountPropertiesMosaicTransaction
	inline AccountPropertiesTransactionBody
	modificationsCount = uint8
	modifications = array(MosaicModification, modificationsCount)

struct AccountPropertiesEntityTypeTransaction
	inline AccountPropertiesTransactionBody
	modificationsCount = uint8
	modifications = array(EntityTypeModification, modificationsCount)
