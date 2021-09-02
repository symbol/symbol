import "mosaic/mosaic_types.cats"
import "transaction.cats"

# binary layout for a mosaic supply change transaction
struct MosaicSupplyChangeTransactionBody
	# affected mosaic identifier
	mosaic_id = UnresolvedMosaicId

	# change amount
	delta = Amount

	# supply change action
	action = MosaicSupplyChangeAction

# binary layout for a non-embedded mosaic supply change transaction
struct MosaicSupplyChangeTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_SUPPLY_CHANGE)

	inline Transaction
	inline MosaicSupplyChangeTransactionBody

# binary layout for an embedded mosaic supply change transaction
struct EmbeddedMosaicSupplyChangeTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_SUPPLY_CHANGE)

	inline EmbeddedTransaction
	inline MosaicSupplyChangeTransactionBody
