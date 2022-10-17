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

# shared content between all verifiable and non-verifiable transfer transactions
inline struct TransferTransactionBody
	TRANSACTION_TYPE = make_const(TransactionType, TRANSFER)

	# [__value__] recipient address
	#
	# [size] recipient address size
	recipient_address = inline SizePrefixedAddress

	# XEM amount
	amount = Amount

	# message envelope size
	@sizeref(message, 4)
	message_envelope_size = uint32

	# optional message
	message = Message if 0 not equals message_envelope_size

# shared content between V0 verifiable and non-verifiable transfer transactions
inline struct TransferTransactionV0Body
	TRANSACTION_VERSION = make_const(uint8, 1)

	inline TransferTransactionBody

# shared content between V1 verifiable and non-verifiable transfer transactions
inline struct TransferTransactionV1Body
	TRANSACTION_VERSION = make_const(uint8, 2)

	inline TransferTransactionBody

	# number of attached mosaics
	mosaics_count = uint32

	# attached mosaics
	# notice that mosaic amount is multipled by transfer amount to get effective amount
	mosaics = array(SizePrefixedMosaic, mosaics_count)

# binary layout for a transfer transaction (V0)
struct TransferTransactionV0
	inline Transaction
	inline TransferTransactionV0Body

# binary layout for a non-verifiable transfer transaction (V0)
struct NonVerifiableTransferTransactionV0
	inline NonVerifiableTransaction
	inline TransferTransactionV0Body

# binary layout for a transfer transaction (V1, latest)
struct TransferTransactionV1
	inline Transaction
	inline TransferTransactionV1Body

# binary layout for a non-verifiable transfer transaction (V1, latest)
struct NonVerifiableTransferTransactionV1
	inline NonVerifiableTransaction
	inline TransferTransactionV1Body
