import "mosaic/mosaic_types.cats"
import "transaction.cats"

# Shared content between MosaicSupplyChangeTransaction and EmbeddedMosaicSupplyChangeTransaction.
inline struct MosaicSupplyChangeTransactionBody
	# Affected mosaic identifier.
	mosaic_id = UnresolvedMosaicId

	# Change amount. It cannot be negative, use the `action` field to indicate if this amount
	# should be **added** or **subtracted** from the current supply.
	delta = Amount

	# Supply change action.
	action = MosaicSupplyChangeAction

# Change the total supply of a mosaic.
struct MosaicSupplyChangeTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_SUPPLY_CHANGE)

	inline Transaction
	inline MosaicSupplyChangeTransactionBody

# Embedded version of MosaicSupplyChangeTransaction.
struct EmbeddedMosaicSupplyChangeTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_SUPPLY_CHANGE)

	inline EmbeddedTransaction
	inline MosaicSupplyChangeTransactionBody
