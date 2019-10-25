import "mosaic/mosaic_types.cats"
import "transaction.cats"

# binary layout for a mosaic definition transaction
struct MosaicDefinitionTransactionBody
	# mosaic identifier
	id = MosaicId

	# mosaic duration
	duration = BlockDuration

	# mosaic nonce
	nonce = MosaicNonce

	# mosaic flags
	flags = MosaicFlags

	# mosaic divisibility
	divisibility = uint8

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

