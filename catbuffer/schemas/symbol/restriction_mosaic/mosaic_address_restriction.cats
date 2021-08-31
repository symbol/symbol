import "transaction.cats"

# binary layout for a mosaic address restriction transaction
struct MosaicAddressRestrictionTransactionBody
	# identifier of the mosaic to which the restriction applies
	mosaicId = UnresolvedMosaicId

	# restriction key
	restrictionKey = uint64

	# previous restriction value
	previousRestrictionValue = uint64

	# new restriction value
	newRestrictionValue = uint64

	# address being restricted
	targetAddress = UnresolvedAddress

# binary layout for a non-embedded mosaic address restriction transaction
struct MosaicAddressRestrictionTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, mosaic_address_restriction)

	inline Transaction
	inline MosaicAddressRestrictionTransactionBody

# binary layout for an embedded mosaic address restriction transaction
struct EmbeddedMosaicAddressRestrictionTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, mosaic_address_restriction)

	inline EmbeddedTransaction
	inline MosaicAddressRestrictionTransactionBody
