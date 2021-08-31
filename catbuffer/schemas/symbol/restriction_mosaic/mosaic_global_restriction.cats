import "restriction_mosaic/restriction_mosaic_types.cats"
import "transaction.cats"

# binary layout for a mosaic global restriction transaction
struct MosaicGlobalRestrictionTransactionBody
	# identifier of the mosaic being restricted
	mosaicId = UnresolvedMosaicId

	# identifier of the mosaic providing the restriction key
	referenceMosaicId = UnresolvedMosaicId

	# restriction key relative to the reference mosaic identifier
	restrictionKey = uint64

	# previous restriction value
	previousRestrictionValue = uint64

	# new restriction value
	newRestrictionValue = uint64

	# previous restriction type
	previousRestrictionType = MosaicRestrictionType

	# new restriction type
	newRestrictionType = MosaicRestrictionType

# binary layout for a non-embedded mosaic global restriction transaction
struct MosaicGlobalRestrictionTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, mosaic_global_restriction)

	inline Transaction
	inline MosaicGlobalRestrictionTransactionBody

# binary layout for an embedded mosaic global restriction transaction
struct EmbeddedMosaicGlobalRestrictionTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, mosaic_global_restriction)

	inline EmbeddedTransaction
	inline MosaicGlobalRestrictionTransactionBody
