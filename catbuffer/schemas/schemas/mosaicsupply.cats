import "transaction.cats"

# supply change directions
enum MosaicSupplyChangeDirection : uint8
	# decreases the supply
	decrease = 0

	# increases the supply
	increase = 1

# binary layout for a mosaic supply change transaction
struct MosaicSupplyChangeTransactionBody
	# id of the affected mosaic
	mosaicId = UnresolvedMosaicId

	# supply change direction
	direction = MosaicSupplyChangeDirection

	# amount of the change
	delta = Amount

# binary layout for a non-embedded mosaic supply change transaction
struct MosaicSupplyChangeTransaction
	const uint8 version = 2
	const EntityType entityType = 0x424D

	inline Transaction
	inline MosaicSupplyChangeTransactionBody

# binary layout for an embedded mosaic supply change transaction
struct EmbeddedMosaicSupplyChangeTransaction
	inline EmbeddedTransaction
	inline MosaicSupplyChangeTransactionBody
