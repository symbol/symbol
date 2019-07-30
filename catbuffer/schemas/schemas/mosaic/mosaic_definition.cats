import "mosaic/mosaic_types.cats"
import "transaction.cats"

# binary layout for a mosaic definition transaction
struct MosaicDefinitionTransactionBody
	# mosaic nonce
	nonce = MosaicNonce

	# mosaic identifier
	id = MosaicId

	# number of elements in optional properties
	propertiesCount = uint8

	# mosaic flags
	flags = MosaicFlags

	# mosaic divisibility
	divisibility = uint8

	# optional properties
	properties = array(MosaicProperty, propertiesCount, sort_key=id)

# binary layout for a non-embedded mosaic definition transaction
struct MosaicDefinitionTransaction
	const uint8 version = 1
	const EntityType entityType = 0x414D

	inline Transaction
	inline MosaicDefinitionTransactionBody

# binary layout for an embedded mosaic definition transaction
struct EmbeddedMosaicDefinitionTransaction
	inline EmbeddedTransaction
	inline MosaicDefinitionTransactionBody

