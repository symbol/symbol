import "restriction_mosaic/restriction_mosaic_types.cats"
import "transaction.cats"

# binary layout for a mosaic global restriction transaction
struct MosaicGlobalRestrictionTransactionBody
	# identifier of the mosaic being restricted
	mosaic_id = UnresolvedMosaicId

	# identifier of the mosaic providing the restriction key
	reference_mosaic_id = UnresolvedMosaicId

	# restriction key relative to the reference mosaic identifier
	restriction_key = uint64

	# previous restriction value
	previous_restriction_value = uint64

	# new restriction value
	new_restriction_value = uint64

	# previous restriction type
	previous_restriction_type = MosaicRestrictionType

	# new restriction type
	new_restriction_type = MosaicRestrictionType

# binary layout for a non-embedded mosaic global restriction transaction
struct MosaicGlobalRestrictionTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_GLOBAL_RESTRICTION)

	inline Transaction
	inline MosaicGlobalRestrictionTransactionBody

# binary layout for an embedded mosaic global restriction transaction
struct EmbeddedMosaicGlobalRestrictionTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_GLOBAL_RESTRICTION)

	inline EmbeddedTransaction
	inline MosaicGlobalRestrictionTransactionBody
