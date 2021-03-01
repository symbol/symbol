import "mosaic/mosaic_types.cats"
import "transaction.cats"

# binary layout for a mosaic supply change transaction
struct MosaicSupplyChangeTransactionBody
	# affected mosaic identifier
	mosaicId = UnresolvedMosaicId

	# change amount
	delta = Amount

	# supply change action
	action = MosaicSupplyChangeAction

# binary layout for a non-embedded mosaic supply change transaction
struct MosaicSupplyChangeTransaction
	const uint8 version = 1
	const EntityType entityType = 0x424D

	inline Transaction
	inline MosaicSupplyChangeTransactionBody

# binary layout for an embedded mosaic supply change transaction
struct EmbeddedMosaicSupplyChangeTransaction
	const uint8 version = 1
	const EntityType entityType = 0x424D

	inline EmbeddedTransaction
	inline MosaicSupplyChangeTransactionBody
