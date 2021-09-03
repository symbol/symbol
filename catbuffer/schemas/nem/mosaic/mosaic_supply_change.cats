import "mosaic.cats"
import "transaction.cats"

# enumeration of mosaic supply change actions
enum MosaicSupplyChangeAction : uint32
	# increases the supply
	INCREASE = 0x01

	# decreases the supply
	DECREASE = 0x02

# binary layout for an mosaic supply change transaction
struct MosaicSupplyChangeTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_SUPPLY_CHANGE)

	inline Transaction

	# mosaic id
	mosaic_id = MosaicId

	# supply change action
	action = MosaicSupplyChangeAction

	# change amount
	delta = Amount
