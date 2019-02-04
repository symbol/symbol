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
