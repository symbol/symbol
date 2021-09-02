import "transaction.cats"

# binary layout for a mosaic address restriction transaction
struct MosaicAddressRestrictionTransactionBody
	# identifier of the mosaic to which the restriction applies
	mosaic_id = UnresolvedMosaicId

	# restriction key
	restriction_key = uint64

	# previous restriction value
	previous_restriction_value = uint64

	# new restriction value
	new_restriction_value = uint64

	# address being restricted
	target_address = UnresolvedAddress

# binary layout for a non-embedded mosaic address restriction transaction
struct MosaicAddressRestrictionTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_ADDRESS_RESTRICTION)

	inline Transaction
	inline MosaicAddressRestrictionTransactionBody

# binary layout for an embedded mosaic address restriction transaction
struct EmbeddedMosaicAddressRestrictionTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_ADDRESS_RESTRICTION)

	inline EmbeddedTransaction
	inline MosaicAddressRestrictionTransactionBody
