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
	const uint8 transaction_version = 1
	const TransactionType transaction_type = mosaic_address_restriction

	inline Transaction
	inline MosaicAddressRestrictionTransactionBody

# binary layout for an embedded mosaic address restriction transaction
struct EmbeddedMosaicAddressRestrictionTransaction
	const uint8 transaction_version = 1
	const TransactionType transaction_type = mosaic_address_restriction

	inline EmbeddedTransaction
	inline MosaicAddressRestrictionTransactionBody
