import "aggregate/cosignature.cats"
import "transaction.cats"

# binary layout for an aggregate transaction
struct AggregateTransactionBody
	# aggregate hash of an aggregate's transactions
	transactionsHash = Hash256

	# transaction payload size in bytes
	# \note this is the total number of bytes occupied by all sub-transactions
	payloadSize = uint32

	# reserved padding to align end of AggregateTransactionHeader on 8-byte boundary
	aggregateTransactionHeader_Reserved1 = uint32

	# sub-transaction data (transactions are variable sized and payload size is in bytes)
	transactions = array(EmbeddedTransaction, size=payloadSize)

	# cosignatures data (fills remaining body space after transactions)
	cosignatures = array(Cosignature, __FILL__)

# binary layout for an aggregate complete transaction
struct AggregateCompleteTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4141

	inline Transaction
	inline AggregateTransactionBody

# binary layout for an aggregate bonded transaction
struct AggregateBondedTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4142

	inline Transaction
	inline AggregateTransactionBody
