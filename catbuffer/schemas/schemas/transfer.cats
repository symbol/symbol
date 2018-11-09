import "transaction.cats"

# binary layout for a transfer transaction
struct TransferTransactionBody
	# transaction recipient
	recipient = UnresolvedAddress
	# size of attached message
	messageSize = uint16
	# number of attached mosaics
	mosaicsCount = uint8
	# transaction message
	message = array(byte, messageSize)
	# attached mosaics
	mosaics = array(UnresolvedMosaic, mosaicsCount, sort_key=mosaicId)

# binary layout for a non-embedded transfer transaction
struct TransferTransaction
	const uint8 version = 3
	const EntityType entityType = 0x4154

	inline Transaction
	inline TransferTransactionBody

# binary layout for an embedded transfer transaction
struct EmbeddedTransferTransaction
	inline EmbeddedTransaction
	inline TransferTransactionBody
