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
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, mosaic_supply_change)

	inline Transaction
	inline MosaicSupplyChangeTransactionBody

# binary layout for an embedded mosaic supply change transaction
struct EmbeddedMosaicSupplyChangeTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, mosaic_supply_change)

	inline EmbeddedTransaction
	inline MosaicSupplyChangeTransactionBody
