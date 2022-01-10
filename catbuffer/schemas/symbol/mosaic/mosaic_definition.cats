import "mosaic/mosaic_types.cats"
import "transaction.cats"

# Shared content between MosaicDefinitionTransaction and Embedded MosaicDefinitionTransaction.
inline struct MosaicDefinitionTransactionBody
	# Unique mosaic identifier obtained from the generator account's public key and the `nonce`.
	#
	# The SDK's can take care of generating this ID for you.
	id = MosaicId

	# Mosaic duration expressed in blocks. If set to 0, the mosaic never expires.
	duration = BlockDuration

	# Random nonce used to generate the mosaic id.
	nonce = MosaicNonce

	# Mosaic flags.
	flags = MosaicFlags

	# Mosaic divisibility.
	divisibility = uint8

# Create a new  [mosaic](/concepts/mosaic.html).
struct MosaicDefinitionTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_DEFINITION)

	inline Transaction
	inline MosaicDefinitionTransactionBody

# Embedded version of MosaicDefinitionTransaction.
struct EmbeddedMosaicDefinitionTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_DEFINITION)

	inline EmbeddedTransaction
	inline MosaicDefinitionTransactionBody

