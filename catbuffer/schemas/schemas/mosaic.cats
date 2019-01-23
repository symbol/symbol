import "transaction.cats"

using MosaicNonce = uint32

# mosaic property flags
enum MosaicFlags : uint8
	# no flags present
	none = 0x00

	# mosaic supply is mutable
	supplyMutable = 0x01

	# mosaic is transferable
	transferable = 0x02

	# mosaic levy is mutable
	levyMutable = 0x04

# available mosaic property ids
enum MosaicPropertyId : uint8
	# mosaic duration
	duration = 2

# mosaic property compose of an id and a value
struct MosaicProperty
	# mosaic property id
	id = MosaicPropertyId

	# mosaic property value
	value = uint64

# binary layout for a mosaic definition transaction
struct MosaicDefinitionTransactionBody
	# mosaic nonce
	mosaicNonce = MosaicNonce

	# id of the mosaic
	mosaicId = MosaicId

	# number of elements in optional properties
	propertiesCount = uint8

	# mosaic flags
	flags = MosaicFlags

	# mosaic divisibility
	divisibility = uint8

	# optional properties
	properties = array(MosaicProperty, propertiesCount)

# binary layout for a non-embedded mosaic definition transaction
struct MosaicDefinitionTransaction
	const uint8 version = 2
	const EntityType entityType = 0x414D

	inline Transaction
	inline MosaicDefinitionTransactionBody

# binary layout for an embedded mosaic definition transaction
struct EmbeddedMosaicDefinitionTransaction
	inline EmbeddedTransaction
	inline MosaicDefinitionTransactionBody

