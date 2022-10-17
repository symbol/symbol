import "transaction.cats"

# Shared content between MosaicAddressRestrictionTransaction and EmbeddedMosaicAddressRestrictionTransaction.
inline struct MosaicAddressRestrictionTransactionBody
	# Identifier of the mosaic to which the restriction applies.
	mosaic_id = UnresolvedMosaicId

	# Restriction key.
	restriction_key = uint64

	# Previous restriction value. Set `previousRestrictionValue` to `FFFFFFFFFFFFFFFF` if
	# the target address does not have a previous restriction value for this mosaic id
	# and restriction key.
	previous_restriction_value = uint64

	# New restriction value.
	new_restriction_value = uint64

	# Address being restricted.
	target_address = UnresolvedAddress

# Set address specific rules to transfer a restrictable mosaic (V1, latest).
struct MosaicAddressRestrictionTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_ADDRESS_RESTRICTION)

	inline Transaction
	inline MosaicAddressRestrictionTransactionBody

# Embedded version of MosaicAddressRestrictionTransaction (V1, latest).
struct EmbeddedMosaicAddressRestrictionTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_ADDRESS_RESTRICTION)

	inline EmbeddedTransaction
	inline MosaicAddressRestrictionTransactionBody
