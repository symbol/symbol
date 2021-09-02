import "mosaic.cats"
import "transaction.cats"

# enumeration of message types
# this is a hint used by the client but ignored by the server
enum MessageType : uint32
	# plain message
	PLAIN = 0x0001

	# encrypted message
	ENCRYPTED = 0x0002

# binary layout for a message
struct Message
	# message type
	message_type = MessageType

	# message size
	message_size = uint32

	# message payload
	message = array(uint8, message_size)

# binary layout for a transfer transaction (V1)
struct TransferTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, TRANSFER)

	inline Transaction

	# recipient address size
	recipient_address_size = make_reserved(uint32, 40)

	# recipient address
	recipient_address = Address

	# XEM amount
	amount = Amount

	# message envelope size
	message_envelope_size = uint32

	# optional message
	message = Message if 0 not equals message_envelope_size

# binary layout for a transfer transaction (V2)
struct TransferTransaction2
	TRANSACTION_VERSION = make_const(uint8, 2)

	inline TransferTransaction

	# number of attached mosaics
	mosaics_count = uint8

	# attached mosaics
	# notice that mosaic amount is multipled by transfer amount to get effective amount
	mosaics = array(Mosaic, mosaics_count, sort_key=mosaic_id)
