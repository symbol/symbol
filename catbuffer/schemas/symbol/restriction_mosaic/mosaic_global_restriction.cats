import "restriction_mosaic/restriction_mosaic_types.cats"
import "transaction.cats"

# Shared content between MosaicGlobalRestrictionTransaction and EmbeddedMosaicGlobalRestrictionTransaction.
inline struct MosaicGlobalRestrictionTransactionBody
	# Identifier of the mosaic being restricted. The mosaic creator must be the signer of the transaction.
	mosaic_id = UnresolvedMosaicId

	# Identifier of the mosaic providing the restriction key.
	# The mosaic global restriction for the mosaic identifier depends on global restrictions set
	# on the reference mosaic. Set `reference_mosaic_id` to **0** if the mosaic giving the
	# restriction equals the `mosaic_id`.
	reference_mosaic_id = UnresolvedMosaicId

	# Restriction key relative to the reference mosaic identifier.
	restriction_key = uint64

	# Previous restriction value.
	previous_restriction_value = uint64

	# New restriction value.
	new_restriction_value = uint64

	# Previous restriction type.
	previous_restriction_type = MosaicRestrictionType

	# New restriction type.
	new_restriction_type = MosaicRestrictionType

# Set global rules to transfer a restrictable mosaic (V1, latest).
struct MosaicGlobalRestrictionTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_GLOBAL_RESTRICTION)

	inline Transaction
	inline MosaicGlobalRestrictionTransactionBody

# Embedded version of MosaicGlobalRestrictionTransaction (V1, latest).
struct EmbeddedMosaicGlobalRestrictionTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_GLOBAL_RESTRICTION)

	inline EmbeddedTransaction
	inline MosaicGlobalRestrictionTransactionBody
