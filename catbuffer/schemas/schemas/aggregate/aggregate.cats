import "transaction.cats"

# binary layout for an aggregate transaction header
struct AggregateTransactionHeader
	# transaction payload size in bytes
	# \note this is the total number of bytes occupied by all sub-transactions
	payloadSize = uint32

# binary layout for an aggregate transaction
struct AggregateTransaction
	const uint8 version = 2
	# aggregate complete
	const EntityType entityType = 0x4141

	inline Transaction
	inline AggregateTransactionHeader

	transactions = array(byte, payloadSize)
	cosignatures = array(byte, size - payloadSize)
