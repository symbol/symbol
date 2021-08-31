import "mosaic/mosaic_types.cats"
import "transaction.cats"

# binary layout for a mosaic definition transaction
struct MosaicDefinitionTransactionBody
	# mosaic identifier
	id = MosaicId

	# mosaic duration
	duration = BlockDuration

	# mosaic nonce
	nonce = MosaicNonce

	# mosaic flags
	flags = MosaicFlags

	# mosaic divisibility
	divisibility = uint8

# binary layout for a non-embedded mosaic definition transaction
struct MosaicDefinitionTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, mosaic_definition)

	inline Transaction
	inline MosaicDefinitionTransactionBody

# binary layout for an embedded mosaic definition transaction
struct EmbeddedMosaicDefinitionTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, mosaic_definition)

	inline EmbeddedTransaction
	inline MosaicDefinitionTransactionBody

