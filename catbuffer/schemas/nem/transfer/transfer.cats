import "mosaic.cats"
import "transaction.cats"

# enumeration of message types
# this is a hint used by the client but ignored by the server
enum MessageType : uint32
	# plain message
	plain = 0x0001

	# encrypted message
	encrypted = 0x0002

# binary layout for a message
struct Message
	# message type
	messageType = MessageType

	# message size
	messageSize = uint32

	# message payload
	message = array(uint8, messageSize)

# binary layout for a transfer transaction (V1)
struct TransferTransaction
	version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, transfer)

	inline Transaction

	# recipient address size
	recipientAddressSize = make_reserved(uint32, 40)

	# recipient address
	recipientAddress = Address

	# XEM amount
	amount = Amount

	# message envelope size
	messageEnvelopeSize = uint32

	# optional message
	message = Message if 0 not equals messageEnvelopeSize

# binary layout for a transfer transaction (V2)
struct TransferTransaction2
	version = make_const(uint8, 2)

	inline TransferTransaction

	# number of attached mosaics
	mosaicsCount = uint8

	# attached mosaics
	# notice that mosaic amount is multipled by transfer amount to get effective amount
	mosaics = array(Mosaic, mosaicsCount, sort_key=mosaicId)
